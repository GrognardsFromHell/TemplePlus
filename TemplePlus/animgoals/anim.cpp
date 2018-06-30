
#include "stdafx.h"

#include <infrastructure/vfs.h>
#include <infrastructure/keyboard.h>
#include "anim.h"
#include "util/fixes.h"
#include "temple_functions.h"
#include "gamesystems/timeevents.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "config/config.h"
#include "obj.h"
#include "critter.h"
#include "pathfinding.h"
#include "party.h"
#include "dice.h"
#include "util/folderutils.h"

#include <map>
#include <set>
#include <fstream>
#include "python/pythonglobal.h"
#include "python/python_debug.h"
#include "python/python_integration_spells.h"
#include "party.h"
#include "action_sequence.h"
#include <temple/meshes.h>

#include <infrastructure/json11.hpp>
#include "combat.h"
#include "location.h"
#include "gamesystems/legacysystems.h"
#include "animgoals_stackentry.h"
#include "animgoals.h"
#include "anim_slot.h"

#define ANIM_RUN_SLOT_CAP 512

BOOL AnimSystem::PushFidget(objHndl handle){

    if (!handle){
        logger->error("PushFidget: Null handle");
        return FALSE;
    }

    auto objBody = objSystem->GetObject(handle);
    if (!objBody)
        return FALSE;

    if (!objBody->IsCritter())
        return FALSE;

    if (!(objBody->GetInt32(obj_f_critter_flags2) & OCF2_AUTO_ANIMATES))
        return FALSE;

    if (combatSys.isCombatActive())
        return FALSE;

    return temple::GetRef<BOOL(__cdecl)(objHndl)>(0x10015D70)(handle);
}


enum AnimGoalDataType {
    AGDT_INT = 0,
    AGDT_OBJ = 1,
    AGDT_LOC = 2
};

static AnimGoalDataType GetGoalPropertyType(AnimGoalProperty property) {
    switch (property) {
    case AGDATA_SELF_OBJ:
        return AGDT_OBJ;
    case AGDATA_TARGET_OBJ:
        return AGDT_OBJ;
    case AGDATA_BLOCK_OBJ: 
        return AGDT_OBJ;
    case AGDATA_SCRATCH_OBJ:
        return AGDT_OBJ;
    case AGDATA_PARENT_OBJ: 
        return AGDT_OBJ;
    case AGDATA_TARGET_TILE: 
        return AGDT_LOC;
    case AGDATA_RANGE_DATA: 
        return AGDT_LOC;
    case AGDATA_ANIM_ID: 
        return AGDT_INT;
    case AGDATA_ANIM_ID_PREV: 
        return AGDT_INT;
    case AGDATA_ANIM_DATA: 
        return AGDT_INT;
    case AGDATA_SPELL_DATA: 
        return AGDT_INT;
    case AGDATA_SKILL_DATA: 
        return AGDT_INT;
    case AGDATA_FLAGS_DATA: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL1: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL2: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL3: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL4: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL5: 
        return AGDT_INT;
    case AGDATA_SCRATCH_VAL6: 
        return AGDT_INT;
    case AGDATA_SOUND_HANDLE: 
        return AGDT_INT;
    }
}

const char *AnimGoalDataNames[] = {
    "AGDATA_SELF_OBJ",     "AGDATA_TARGET_OBJ",   "AGDATA_BLOCK_OBJ",
    "AGDATA_SCRATCH_OBJ",  "AGDATA_PARENT_OBJ",   "AGDATA_TARGET_TILE",
    "AGDATA_RANGE_DATA",   "AGDATA_ANIM_ID",      "AGDATA_ANIM_ID_PREV",
    "AGDATA_ANIM_DATA",    "AGDATA_SPELL_DATA",   "AGDATA_SKILL_DATA",
    "AGDATA_FLAGS_DATA",   "AGDATA_SCRATCH_VAL1", "AGDATA_SCRATCH_VAL2",
    "AGDATA_SCRATCH_VAL3", "AGDATA_SCRATCH_VAL4", "AGDATA_SCRATCH_VAL5",
    "AGDATA_SCRATCH_VAL6", "AGDATA_SOUND_HANDLE"};

static void assertStructSizes() {
  static_assert(temple::validate_size<ObjectId, 0x18>::value,
                "Goal Stack entry has incorrect size.");
  static_assert(temple::validate_size<AnimSlot, 0x2C98>::value,
                "AnimSlot has incorrect size");
}

static class AnimAddressTable : temple::AddressTable {
public:
  uint32_t *nextUniqueId;
  uint32_t *slotsInUse;

  /*
  Some goal states want to set a dynamic delay for transitioning into the next
  state. They use this global variable to transport this value into the state
  machine controller below.
  */
  int *customDelayInMs;

  void(__cdecl *GetAnimName)(int animId, char *animNameOut);
  void(__cdecl *PushGoalDying)(objHndl obj, int rotation);
  void(__cdecl *InterruptAllForTbCombat)();

  int (*anim_first_run_idx_for_obj)(objHndl obj);
  BOOL (*anim_run_id_for_obj)(objHndl obj, AnimSlotId *slotIdOut);
  BOOL(*anim_frame_advance_maybe)(objHndl, AnimSlot& runSlot, uint32_t animHandle, uint32_t* eventOut);
  AnimSlotId tmpId;

  AnimAddressTable() {
    rebase(GetAnimName, 0x102629D0);
    rebase(nextUniqueId, 0x11E61520);
    rebase(slotsInUse, 0x10AA4BBC);
    rebase(customDelayInMs, 0x10307534);

    rebase(PushGoalDying, 0x100157B0);
    rebase(anim_frame_advance_maybe, 0x10016530);
    rebase(InterruptAllForTbCombat, 0x1000C950);

    rebase(anim_first_run_idx_for_obj, 0x10054E20);
    rebase(anim_run_id_for_obj, 0x1000C430);
  }

} animAddresses;

static const uint32_t TRANSITION_LOOP = 0x10000000;
static const uint32_t TRANSITION_END = 0x20000000;
static const uint32_t TRANSITION_GOAL = 0x40000000;
static const uint32_t TRANSITION_UNK1 = 0x90000000;

static struct AnimationAdresses : temple::AddressTable {

  bool(__cdecl *PushRotate)(objHndl obj, float rotation);

  bool(__cdecl *PushUseSkillOn)(objHndl actor, objHndl target,
                                objHndl scratchObj, SkillEnum skill,
                                int goalFlags);

  int(__cdecl *PushAttackAnim)(objHndl actor, objHndl target, int unk1,
                               int hitAnimIdx, int playCrit,
                               int useSecondaryAnim);

  bool(__cdecl *PushRunNearTile)(objHndl actor, LocAndOffsets target,
                                 int radiusFeet);

  bool(__cdecl *PushUnconceal)(objHndl actor);

  bool(__cdecl *Interrupt)(objHndl actor, AnimGoalPriority priority, bool all);

  void(__cdecl *PushFallDown)(objHndl actor, int unk);

  int(__cdecl *GetAnimIdSthgSub_1001ABB0)(objHndl actor);

  int(__cdecl *PushAttemptAttack)(objHndl, objHndl);

  int(__cdecl *PushAnimate)(objHndl obj, int anim);

  AnimSlotId *animIdGlobal;

  void(__cdecl*Debug)();

  AnimationAdresses() {

    rebase(Interrupt, 0x1000C7E0);
    rebase(PushAnimate, 0x10015290);
    rebase(PushRotate, 0x100153E0);
    rebase(PushFallDown, 0x100157B0);
    rebase(PushUnconceal, 0x10015E00);

    rebase(PushAttemptAttack, 0x1001A540);
    rebase(GetAnimIdSthgSub_1001ABB0, 0x1001ABB0);

    rebase(PushUseSkillOn, 0x1001C690);
    rebase(PushRunNearTile, 0x1001C1B0);

    rebase(PushAttackAnim, 0x1001C370);

    rebase(animIdGlobal, 0x102AC880);

    rebase(Debug, 0x10055130);
  }

} addresses;

class GoalStateFuncs
{
#define GoalStateFunc(fname) static int __cdecl GoalStateFunc ## fname ## (AnimSlot& slot);
public:

    static int __cdecl GoalStateFunc35(AnimSlot& ars);
    static int __cdecl GoalStateFunc54(AnimSlot& slot); // used in ag_attempt_attack
    static int __cdecl GoalStateFunc133(AnimSlot& slot);
    static int __cdecl GoalStateFuncIsCritterProne(AnimSlot& slot);
    static int __cdecl GoalStateFuncIsParam1Concealed(AnimSlot& slot);
    static int __cdecl GoalStateFunc78_IsHeadingOk(AnimSlot &slot);
    static int __cdecl GoalStateFunc82(AnimSlot& slot);
    static int __cdecl GoalStateFunc83(AnimSlot& slot);
    static int __cdecl GoalStateFunc65(AnimSlot& slot); // belongs in ag_hit_by_weapon
    static int __cdecl GoalStateFunc70(AnimSlot& slot); // 
    static int __cdecl GoalStateFunc100_IsCurrentPathValid(AnimSlot& slot);
    static int __cdecl GoalStateFunc106(AnimSlot& slot);
    static int __cdecl GoalStateFunc130(AnimSlot& slot);
    static int __cdecl GoalStateFuncPickpocket(AnimSlot &slot);
    
    static float __cdecl HookedGetRunSpeed(objHndl handle, obj_f field);
} gsFuncs;

bool AnimSystem::PushRotate(objHndl obj, float rotation) {
  return addresses.PushRotate(obj, rotation);
}

bool AnimSystem::PushUseSkillOn(objHndl actor, objHndl target,
                                    SkillEnum skill, objHndl scratchObj,
                                    int goalFlags) {
  return addresses.PushUseSkillOn(actor, target, scratchObj, skill, goalFlags);
}

bool AnimSystem::PushRunNearTile(objHndl actor, LocAndOffsets target,
                                     int radiusFeet) {
  return addresses.PushRunNearTile(actor, target, radiusFeet);
}

bool AnimSystem::PushRunToTile(objHndl handle, LocAndOffsets loc, PathQueryResult * pqr)
{

    //return temple::GetRef<BOOL(__cdecl)(objHndl, LocAndOffsets, PathQueryResult*)>(0x1001A2E0)(handle, loc, pqr);
    
    if (!handle
        || critterSys.IsDeadOrUnconscious(handle) 
        || (!critterSys.IsPC(handle) && temple::GetRef<objHndl(__cdecl)(objHndl)>(0x10053CA0)(handle))  ) // npc is conversing with pc
        return false;


    AnimSlotGoalStackEntry goalData(handle, ag_run_to_tile);
    AnimSlot *runSlot;
    AnimSlotId * animIdGlobal = addresses.animIdGlobal;

    if (gameSystems->GetAnim().DoesObjHaveGoal(handle, ag_run_to_tile, animIdGlobal)){ // is obj doing related goal?
        goalData.targetTile.location = loc;
        if ( !Interrupt(handle, AnimGoalPriority::AGP_3, 0)
             || !goalData.Push(animIdGlobal)
             || !GetSlot(animIdGlobal, &runSlot)){
            return true;
        };
        if (!pqr) {
            runSlot->path.flags &= ~PF_COMPLETE;
        }
    } 
    else{
        if (pqr) {
            goalData.targetTile.location = pqr->to;
        }
        else {
            goalData.targetTile.location = loc;
        }
        if (!Interrupt(handle, AnimGoalPriority::AGP_3, 0) || !goalData.Push(animIdGlobal))
            return false;
        if (!GetSlot(animIdGlobal, &runSlot)) {
            return true;
        }
        if (!pqr) {
            runSlot->path.flags = 0;
        }
    }
    
    if (pqr) {
        *(Path*)(&runSlot->path) = *(Path*)pqr;
        GoalDestinationAdd(runSlot->path.mover, runSlot->path.to);
        runSlot->field_14 = 0;
    }
    else {
        GoalDestinationRemove(runSlot->path.mover);
    }

    return true;
}

bool AnimSystem::ShouldRun(objHndl handle){

    auto isAlwaysRun = config.GetVanillaInt("always run") != 0;
    if (config.walkDistanceFt > 0) // adding support for "Walk Distance" option
        isAlwaysRun = false;

    auto isInParty = party.IsInParty(handle);
    if (!isInParty)
        return isAlwaysRun;

    auto isCtrlPressed = infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL);

    if (isAlwaysRun) {
        if (isCtrlPressed || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL))
            return false;
    }
    else {
        if (isCtrlPressed || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL))
            return true;
        if (config.walkDistanceFt == 0 && infrastructure::gKeyboard.IsModifierActive(VK_NUMLOCK))
            return true;
    }

    return isAlwaysRun;
}

bool AnimSystem::PushMoveToTile(objHndl handle, LocAndOffsets loc){

    if (!handle || critterSys.IsDeadOrUnconscious(handle))
        return FALSE;

    if (!critterSys.IsPC(handle)) {
        auto getDlgTarget = temple::GetRef<objHndl(__cdecl)(objHndl)>(0x10053CA0);
        if (getDlgTarget(handle) != objHndl::null)
            return FALSE;
    }

    // if (critterSys.IsPC(handle) && ShouldRun(handle)) {
    if (party.IsInParty(handle) && ShouldRun(handle)) {
        return PushRunToTile(handle, loc);
    }

    auto animIdGlobal = temple::GetPointer<AnimSlotId>(0x102AC880);
    if (!gameSystems->GetAnim().DoesObjHaveGoal(handle, ag_move_to_tile, animIdGlobal)) {
        AnimSlotGoalStackEntry goalData(handle, ag_move_to_tile);
        goalData.targetTile.location = loc;
        if (Interrupt(handle, AnimGoalPriority::AGP_3, false))
            if (goalData.Push(animIdGlobal)) {
                return true;
            }
        return false;
    }

    AnimSlotGoalStackEntry goalData(handle, ag_move_to_tile);
    goalData.targetTile.location = loc;
    if (!Interrupt(handle, AnimGoalPriority::AGP_3, false))
        return true;
    goalData.Push(animIdGlobal);
    return true;
}

bool AnimSystem::PushWalkToTile(objHndl handle, LocAndOffsets loc){
    PushMoveToTile(handle, loc);
    AnimSlotId *runId = addresses.animIdGlobal;
    AnimSlot *runSlot;
    GetSlot(runId, &runSlot);
    runSlot->path.flags &= ~PF_COMPLETE;
    GoalDestinationRemove(handle);
    SetRunningState(false);
    return false;
}

void AnimSystem::SetRunningState(bool state){
    AnimSlotId *runId = addresses.animIdGlobal;
    AnimSlot *runInfo;
    if (GetSlot(runId, &runInfo)) {
        if (state) {
            runInfo->flags |= ASF_RUNNING;
        }
        else {
            runInfo->flags &= ~ASF_RUNNING;
        }
    }
}

void AnimSystem::TurnOnRunning(){
    SetRunningState(true);
}

bool AnimSystem::PushUnconceal(objHndl actor) {
  return addresses.PushUnconceal(actor);
}

void AnimSystem::PushFallDown(objHndl actor, int unk) {
  addresses.PushFallDown(actor, unk);
}

int AnimSystem::PushAttackAnim(objHndl actor, objHndl target, int unk1,
                                   int hitAnimIdx, int playCrit,
                                   int useSecondaryAnim) {
  return addresses.PushAttackAnim(actor, target, unk1, hitAnimIdx, playCrit,
                                  useSecondaryAnim);
}

int AnimSystem::GetActionAnimId(objHndl objHndl) {
  return addresses.GetAnimIdSthgSub_1001ABB0(objHndl);
}

int AnimSystem::PushAttemptAttack(objHndl attacker, objHndl defender) {
  return addresses.PushAttemptAttack(attacker, defender);
}

int AnimSystem::PushDodge(objHndl attacker, objHndl dodger){
    return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x100158E0)(attacker, dodger);
}

int AnimSystem::PushAnimate(objHndl obj, int anim) {
  return addresses.PushAnimate(obj, anim);
}

BOOL AnimSystem::PushSpellCast(SpellPacketBody & spellPkt, objHndl item)
{

    // note: the original included the spell ID generation & registration, this is separated here.
    auto caster = spellPkt.caster;
    auto casterObj = objSystem->GetObject(caster);
    AnimSlotGoalStackEntry goalData;
    if (!goalData.InitWithInterrupt(caster, ag_throw_spell_w_cast_anim))
        return FALSE;

    goalData.skillData.number = spellPkt.spellId;

    SpellEntry spEntry(spellPkt.spellEnum);

    // if self-targeted spell
    if (spEntry.IsBaseModeTarget(UiPickerType::Single) && spellPkt.targetCount == 0 ){
        goalData.target.obj = spellPkt.caster;
        
        if (spellPkt.aoeCenter.location.location == 0){
            goalData.targetTile.location = casterObj->GetLocationFull();
        } 
        else{
            goalData.targetTile.location = spellPkt.aoeCenter.location;
        }
    } 

    else{
        auto tgt = spellPkt.targetListHandles[0];
        goalData.target.obj = tgt;
        if (tgt && spellPkt.aoeCenter.location.location == 0 ){
            goalData.targetTile.location = objSystem->GetObject(tgt)->GetLocationFull();
        }
        else {
            goalData.targetTile.location = spellPkt.aoeCenter.location;
        }
    }

    if (inventory.UsesWandAnim(item)){
        goalData.animIdPrevious.number = temple::GetRef<int(__cdecl)(int)>(0x100757C0)(spEntry.spellSchoolEnum); // GetAnimIdWand	
    }
    else{
        goalData.animIdPrevious.number = temple::GetRef<int(__cdecl)(int)>(0x100757B0)(spEntry.spellSchoolEnum); // GetSpellSchoolAnimId
    }

    AnimSlotId slotIdNew;
    return goalData.Push(&slotIdNew);
}

BOOL AnimSystem::PushSpellInterrupt(const objHndl& caster, objHndl item, AnimGoalType animGoalType, int spellSchool){
    AnimSlotGoalStackEntry goalData;
    goalData.InitWithInterrupt(caster, animGoalType);
    goalData.target.obj = (*actSeqSys.actSeqCur)->spellPktBody.caster;
    goalData.skillData.number = 0;
    if (inventory.UsesWandAnim(item))
        goalData.animIdPrevious.number = temple::GetRef<int(__cdecl)(int)>(0x100757C0)(spellSchool); // GetAnimIdWand	
    else
        goalData.animIdPrevious.number = temple::GetRef<int(__cdecl)(int)>(0x100757B0)(spellSchool); // GetSpellSchoolAnimId
    
    AnimSlotId idNew;
    return goalData.Push(&idNew);
}

void AnimSystem::PushForMouseTarget(objHndl handle, AnimGoalType type, objHndl tgt, locXY loc, objHndl scratchObj, int someFlag){

    AnimSlotGoalStackEntry goalData;

    temple::GetRef<void(__cdecl)(AnimSlotGoalStackEntry&, objHndl, AnimGoalType, objHndl, locXY, objHndl, int)>(0x10113470)(goalData, handle, type, tgt, loc, scratchObj, someFlag);
    return;

    // rewriting function in p[rogress..
    if (!goalData.Init(handle, type))
        return;
    goalData.target.obj = tgt;

    
    if (loc) {
        goalData.targetTile.location.location = loc;
        goalData.targetTile.location.off_x = goalData.targetTile.location.off_y = 0;
    }
    if (scratchObj) {
        goalData.scratch.obj = scratchObj;
    }
    
    static auto isActionPaused = [](objHndl handle, int actionIdx) {
        if (!handle)
            return false;
        auto critterFlags2 = objects.getInt32(handle, obj_f_critter_flags2);
        return ((CritterFlags2::OCF2_ACTION0_PAUSED << actionIdx) & critterFlags2) != 0;
    };
    if (type == ag_attack || type == ag_attempt_attack || isActionPaused(handle, 0)) {
            return;
    }

    /*if (!combatSys.IsAutoAttack()){
        if (gameSystems->GetAnim().())
    }*/
    
}

void AnimSystem::Debug(){
    gameSystems->GetAnim().DebugGoals();
}

const AnimGoal* AnimSystem::GetGoal(AnimGoalType goalType)
{
    return &animationGoals_->GetByType(goalType);
}

struct GoalDestination {
    objHndl handle;
    LocAndOffsets loc;
};
const int sGoalDestinationCap = 20;

void AnimSystem::GoalDestinationRemove(objHndl handle){
    auto gd = temple::GetRef<GoalDestination[]>(0x10BCA8C0);
    for (auto i = 0; i < sGoalDestinationCap; i++) {
        if (gd[i].handle == handle) {
            gd[i].handle = objHndl::null;
        }
    }
}

void AnimSystem::GoalDestinationAdd(objHndl handle, LocAndOffsets loc){
    auto gd = temple::GetRef<GoalDestination[]>(0x10BCA8C0);
    auto &gdIdx = temple::GetRef<int>(0x10BCAAA0);
    gd[gdIdx].handle = handle;
    gd[gdIdx].loc = loc;
    gdIdx++;
    if (gdIdx == sGoalDestinationCap)
        gdIdx = 0;
}

void AnimSystem::SetRuninfoDeallocCallback(void(* cb)()){
    temple::GetRef<void(__cdecl*)()>(0x10AA4BB4) = cb;
}

bool AnimSystem::InterruptAllForTbCombat(){
    static auto interruptAllForTbCombat = temple::GetRef<BOOL(__cdecl)()>(0x1000C950);
    return interruptAllForTbCombat();
}

std::optional<AnimSlotId> AnimSystem::AllocSlot()
{
	// Find a free slot
	int freeSlot = -1;
	for (int i = 0; i < ANIM_RUN_SLOT_CAP; i++) {
		if (!mSlots[i].IsActive()) {
			freeSlot = i;
			break;
		}
	}
	if (freeSlot == -1) {
		logger->error("All animation slots are in use!");
		mAllSlotsUsed = true;
		return {};
	}

	auto &slot = mSlots[freeSlot];
	slot.id.slotIndex = freeSlot;
	slot.id.uniqueId = (*animAddresses.nextUniqueId)++;
	slot.id.field_8 = 0;
	slot.flags = 1;
	slot.animPath.pathLength = 0;
	slot.path.flags = 0;
	slot.pCurrentGoal = 0;
	slot.animObj = objHndl::null;
	slot.currentState = -1;
	slot.nextTriggerTime.timeInDays = 0;
	slot.nextTriggerTime.timeInMs = 0;

	slot.goals[0].self.obj = objHndl::null;
	slot.goals[0].target.obj = objHndl::null;
	slot.goals[0].block.obj = objHndl::null;
	slot.goals[0].scratch.obj = objHndl::null;
	slot.goals[0].parent.obj = objHndl::null;
	slot.goals[0].targetTile.obj = objHndl::null;
	slot.goals[0].selfTracking.guid = ObjectId::CreateNull();
	slot.goals[0].targetTracking.guid = ObjectId::CreateNull();
	slot.goals[0].blockTracking.guid = ObjectId::CreateNull();
	slot.goals[0].scratchTracking.guid = ObjectId::CreateNull();
	slot.goals[0].parentTracking.guid = ObjectId::CreateNull();

	++(*animAddresses.slotsInUse);
	
	return slot.id;
}

BOOL AnimSystem::GetSlot(AnimSlotId * runId, AnimSlot **runSlotOut) {
	*runSlotOut = gameSystems->GetAnim().GetSlot(*runId);
	return (*runSlotOut != nullptr) ? 1 : 0;
}



//*****************************************************************************
//* Anim
//*****************************************************************************

BOOL AnimPathSpec::PathSearch(){
    return temple::GetRef<BOOL(__cdecl)(AnimPathSpec*)>(0x1003FCA0)(this);
}

AnimSystem::AnimSystem(const GameSystemConf &config) {

  animationGoals_ = std::make_unique<AnimationGoals>();

  auto startup = temple::GetPointer<int(const GameSystemConf *)>(0x10016bb0);
  if (!startup(&config)) {
    throw TempleException("Unable to initialize game system Anim");
  }
}
AnimSystem::~AnimSystem() {
  auto shutdown = temple::GetPointer<void()>(0x1000c110);
  shutdown();
}
void AnimSystem::Reset() {
  auto reset = temple::GetPointer<void()>(0x1000c120);
  reset();
}
bool AnimSystem::SaveGame(TioFile *file) {
  auto save = temple::GetPointer<int(TioFile *)>(0x1001cab0);
  return save(file) == 1;
}
bool AnimSystem::LoadGame(GameSystemSaveFile *saveFile) {
  auto load = temple::GetPointer<int(GameSystemSaveFile *)>(0x1001d250);
  return load(saveFile) == 1;
}
const std::string &AnimSystem::GetName() const {
  static std::string name("Anim");
  return name;
}

void AnimSystem::ClearGoalDestinations() {
  static auto clear = temple::GetPointer<void()>(0x100BACC0);
  clear();
}

void AnimSystem::InterruptAll() {
  static auto anim_interrupt_all = temple::GetPointer<BOOL()>(0x1000c890);
  anim_interrupt_all();
}

BOOL AnimSystem::ProcessAnimEvent(const TimeEvent *evt) {

  if (mAllSlotsUsed) {
	InterruptAllGoalsUpToPriority(AGP_3);
    mAllSlotsUsed = FALSE;
  }

  // The animation slot id we're triggered for
  AnimSlotId triggerId = {evt->params[0].int32, evt->params[1].int32,
                          evt->params[2].int32};

  assert(triggerId.slotIndex < 512);

  auto &slot = mSlots[triggerId.slotIndex];

  // This seems like a pretty stupid check since slots cannot "move"
  // and the first part of their ID must be the slot index
  // Shouldn't this really check for the unique id of the animation instead?
  if (slot.id.slotIndex != triggerId.slotIndex) {
    logger->debug("{} != {}", slot.id, triggerId);
    return TRUE;
  }

  // Slot did belong to "us", but it was deactivated earlier
  if (!slot.IsActive()) {
    ProcessActionCallbacks();
    return TRUE;
  }

  // Interesting how this reschedules in at least 100ms which seems steep for
  // animation processing
  // Have to check where and why this is set
  if (slot.flags & ASF_UNK1) {
    ProcessActionCallbacks();

    auto delay = std::max(slot.path.someDelay, 100);
    
    return RescheduleEvent(delay, slot, evt);
  }

  if (slot.IsStackEmpty()) {
    logger->warn("Found slot {} with goal < 0", slot.id);
    slot.currentGoal = 0;
  }

  // This sets the current stack pointer, although it should already be set.
  // They used
  // a lot of safeguard against themselves basically
  auto currentGoal = &slot.goals[slot.currentGoal];
  slot.pCurrentGoal = currentGoal;

  bool stopProcessing = false;
  const AnimGoal *goal = nullptr;
 // auto oldGoal = goal;

  // And another safeguard
  if (currentGoal->goalType < 0 || currentGoal->goalType >= ag_count) {
    slot.flags |= ASF_STOP_PROCESSING;
    stopProcessing = true;
  } else {
	goal = &animationGoals_->GetByType(currentGoal->goalType);
  }

  // This validates object references found in the animation slot
  if (!PrepareSlotForGoalState(slot, nullptr)) {
    ProcessActionCallbacks();
    return TRUE;
  }

  // Validates that the object the animation runs for is not destroyed
  if (slot.animObj) {
    if (objects.GetFlags(slot.animObj) & OF_DESTROYED) {
      logger->warn("Processing animation slot {} for destroyed object.",
                   slot.id);
    }
  } else {
    // Animation is no longer associated with an object after validation
    slot.flags |= ASF_STOP_PROCESSING;
    stopProcessing = true;
  }

  int delay = 0;
  mCurrentlyProcessingSlotIdx = slot.id.slotIndex;
  // TODO: Clean up this terrible control flow
  if (!stopProcessing) {

    mCurrentlyProcessingSlotIdx = slot.id.slotIndex;

    // TODO: processing
    int loopNr = 0;

    while (!stopProcessing) {
      ++loopNr;

      // This only applies to in-development i think, since as of now there
      // should be no infi-looping goals
      if (loopNr >= 100) {
        logger->error("Goal {} loops infinitely in animation {}!",
                      slot.pCurrentGoal->goalType, slot.id);
        combatSys.CombatAdvanceTurn(slot.animObj);
        mCurrentlyProcessingSlotIdx = -1;
        InterruptGoals(slot, AGP_HIGHEST);
        ProcessActionCallbacks();
        return TRUE;
      }

      auto &currentState = goal->states[slot.currentState];

      // Prepare for the current state
      if (!PrepareSlotForGoalState(slot, &currentState)) {
        ProcessActionCallbacks();
        return TRUE;
      }

      /*


      *******  PROCESSING *******
      
      
      */

      auto stateResult = currentState.callback(slot);

      // Check flags on the slot that may have been set by the callbacks.
      if (slot.flags & ASF_UNK1) {
        stopProcessing = true;
      }

      if (!(slot.flags & ASF_ACTIVE)) {
        mCurrentlyProcessingSlotIdx = -1;
        ProcessActionCallbacks();
        return TRUE;
      }

      if (slot.flags & ASF_STOP_PROCESSING) {
        break;
      }

      auto &transition =
          stateResult ? currentState.afterSuccess : currentState.afterFailure;
      auto nextState = transition.newState;
      delay = transition.delay;

      // Special transitions
      if (nextState & ASTF_MASK) {
        /*  if (currentGoal->goalType != ag_anim_idle)
            logger->debug("ProcessAnimEvent: Special transition; currentState: {:x}", slot.currentState);*/
        if (nextState & ASTF_REWIND) {
            /*if (currentGoal->goalType != ag_anim_idle)
                logger->debug("Setting currentState to 0");*/
          slot.currentState = 0;
          stopProcessing = true;
        }

        if ((nextState & ASTF_POP_GOAL_TWICE) == ASTF_POP_GOAL_TWICE) {
        //	logger->debug("Popping 2 goals due to 0x38000000");
            auto newGoal = &goal;
            auto popFlags = nextState;
            PopGoal(slot, popFlags, newGoal, &currentGoal, &stopProcessing);
            PopGoal(slot, popFlags, newGoal, &currentGoal, &stopProcessing);
            //oldGoal = goal;
        } 
        else if ( (nextState & ASTF_POP_GOAL) == ASTF_POP_GOAL) {
        //  logger->debug("Popping 1 goals due to 0x30000000");
          auto newGoal = &goal;
          auto popFlags = nextState;
          PopGoal(slot, popFlags, newGoal, &currentGoal, &stopProcessing);
          //oldGoal = goal;
        }

        if (nextState & ASTF_PUSH_GOAL) {
            if (slot.IsStackFull()) {
                logger->error("Unable to push goal, because anim slot %s has overrun!", slot.id.ToString());
                logger->error("Current sub goal stack is:");
                
                for (auto i = 0; i < slot.currentGoal; i++) {
                    logger->info("\t[{}]: Goal {}", i, slot.goals[i].goalType);
                }

                //oldGoal = goal;
                slot.flags |= ASF_STOP_PROCESSING;
                slot.currentState = 0;
                stopProcessing = true;
            } 
            else {
                slot.currentState = 0;
                slot.currentGoal++;

                currentGoal = &slot.goals[slot.currentGoal];
                slot.pCurrentGoal = currentGoal;

                // Apparently if 0x30 00 00 00 is also set, it copies the previous goal????
                if (slot.currentGoal > 0 && (nextState & ASTF_POP_GOAL) != ASTF_POP_GOAL) {
                //	logger->debug("Copying previous goal");
                    slot.goals[slot.currentGoal] = slot.goals[slot.currentGoal - 1];
                }

                auto newGoalType = static_cast<AnimGoalType>(nextState & 0xFFF);
				goal = &animationGoals_->GetByType(newGoalType);
                slot.goals[slot.currentGoal].goalType = newGoalType;

                IncreaseActiveGoalCount(slot, *goal);
            }
        }
        if ((nextState & ASTF_POP_ALL) == ASTF_POP_ALL) {
        //  logger->debug("ProcessAnimEvent: 0x90 00 00 00");
          currentGoal = &slot.goals[0];
		  goal = &animationGoals_->GetByType(slot.goals[0].goalType);
		  auto prio = goal->priority;
          if (prio < AnimGoalPriority::AGP_7) {
            //    logger->debug("ProcessAnimEvent: root goal priority less than 7");
                slot.flags |= AnimSlotFlag::ASF_STOP_PROCESSING;
            
                for (auto i = 1; i < slot.currentGoal; i++) {
					auto _goal = &animationGoals_->GetByType(slot.goals[i].goalType);

                    if (_goal->state_special.callback) {
                        if (PrepareSlotForGoalState(slot, &_goal->state_special)) {
                          _goal->state_special.callback(slot);
                        }
                    }
                }

				auto goal0 = &animationGoals_->GetByType(slot.goals[0].goalType);
                if (goal0->state_special.callback) {
                  if (PrepareSlotForGoalState(slot, &goal0->state_special))
                    goal0->state_special.callback(slot);
                }
                slot.animPath.flags |= 1u;
                slot.currentState = 0;
                slot.path.flags &= ~PF_COMPLETE;
                GoalDestinationsRemove(slot.path.mover);
                //oldGoal = goal;
                slot.field_14 = -1;
                stopProcessing = true;
            } 
            else {
            //	logger->debug("ProcessAnimEvent: root goal priority equal to 7");
                currentGoal = &slot.goals[slot.currentGoal];
				goal = &animationGoals_->GetByType(currentGoal->goalType);
                // oldGoal = goal;
                while (goal->priority < AnimGoalPriority::AGP_7) {
                  PopGoal(slot, ASTF_POP_GOAL, &goal, &currentGoal, &stopProcessing);
                //  logger->debug("ProcessAnimEvent: Popped goal for {}.", description.getDisplayName(slot.animObj));
                  currentGoal = &slot.goals[slot.currentGoal];
				  goal = &animationGoals_->GetByType(currentGoal->goalType);
                  // oldGoal = goal;
                }
            }
        }
      } else {
        // Normal jump to another state without special flags
        --nextState; // Jumps are 1-based, although the array is 0-based
        if (slot.currentState == nextState) {
          logger->error("State {} of goal {} transitioned into itself.",
                        slot.currentState, currentGoal->goalType);
        }
        slot.currentState = nextState;
      }

      if (delay) {
        switch (delay) {
        case AnimStateTransition::DelaySlot:
          // Use the delay specified in the slot. Reasoning currently unknown.
          // NOTE: Could mean that it's waiting for pathing to complete
          delay = slot.path.someDelay;
          break;
        case AnimStateTransition::DelayCustom:
          // Used by some goal states to set their desired dynamic delay
          delay = *animAddresses.customDelayInMs;
          break;
        case AnimStateTransition::DelayRandom:
          // Calculates the animation delay randomly in a range from 0 to 300
          delay = RandomIntRange(0, 300);
          break;
        default:
          // Keep predefined delay
          break;
        }

        stopProcessing = true;
      }

      // If no delay has been set, the next state is immediately processed
    }
  }

  mCurrentlyProcessingSlotIdx = -1;
  AnimSlotFlag slotFlags = static_cast<AnimSlotFlag>(slot.flags); // for debug
  // Does Flag 2 mean "COMPLETED" ?
  if (!(slot.flags & ASF_STOP_PROCESSING)) {
    if (slot.flags & ASF_ACTIVE) {
      // This actually seems to be the "GOOD" case
      auto result = RescheduleEvent(delay, slot, evt);
      ProcessActionCallbacks();
      if (slot.pCurrentGoal->goalType != ag_anim_idle)  {
        //  logger->debug("ProcessAnimEvent: rescheduled for {}, goal {}", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType]);
      } else
      {
          int da = 1;
      }
      return result;
    }
    ProcessActionCallbacks();
    return TRUE;
  }

  auto result = TRUE;
  if (slot.animObj) {

    // preserve the values i case the slot gets deallocated below
    auto animObj = slot.animObj;
    auto actionAnimId = slot.uniqueActionId;
    //logger->debug("ProcessAnimEvent: Interrupting goals.");
    // Interrupt everything for the slot
    result = InterruptGoals(slot, AGP_HIGHEST);

    if (!slot.animObj)
    {
        int aha = 0;
    }

    if (animObj && objects.IsCritter(animObj)) {
      //PushActionCallback(slot);
        if (!actionAnimId)
        {
            int dummy = 1;
        }
        mActionCallbacks.push_back({ animObj, actionAnimId });
    }
  }

  ProcessActionCallbacks();
  return result;
}

void AnimSystem::PushDisableFidget()
{
    static auto call = temple::GetPointer<void()>(0x100603F0);
    call();
}

void AnimSystem::PopDisableFidget()
{
    static auto call = temple::GetPointer<void()>(0x10060410);
    call();
}

void AnimSystem::StartFidgetTimer()
{
    static auto queue_fidget_anim_event = temple::GetPointer<BOOL()>(0x100146c0);
    queue_fidget_anim_event();
}

// Originally @ 0x10054E20
int AnimSystem::GetFirstRunSlotIdxForObj(objHndl handle) const {
    for (int i = 0; i < AnimSlotCount; i++) {
        auto &slot = mSlots[i];

        if (slot.IsActive()
            && !slot.IsStopProcessing()
            && slot.currentGoal > -1
            && slot.id.slotIndex != -1
            && slot.animObj == handle) {
            return i;
        }
    }

    return -1;
}

// Originally @ 0x10054E70
int AnimSystem::GetNextRunSlotIdxForObj(objHndl handle, int startSlot) const {
    for (int i = startSlot + 1; i < AnimSlotCount; i++) {
        auto &slot = mSlots[i];

        if (slot.IsActive()
            && !slot.IsStopProcessing()
            && slot.currentGoal > -1
            && slot.id.slotIndex != -1
            && slot.animObj == handle) {
            return i;
        }
    }

    return -1;
}

AnimSlot& AnimSystem::GetRunSlot(int slotId) const {
    return mSlots[slotId];
}

// Originally @ 0x1000C430
std::optional<AnimSlotId> AnimSystem::GetFirstRunSlotId(objHndl handle) const {
    
    for (auto slotIdx = GetFirstRunSlotIdxForObj(handle); 
        slotIdx != -1; 
        slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {

        auto &slot = GetRunSlot(slotIdx);

		auto goal = &animationGoals_->GetByType(slot.goals[0].goalType);
        if (!goal->interruptAll) {
            return slot.id;
        }
    }

    return {};

}

// Originally @ 0x10054fd0
std::optional<AnimSlotId> AnimSystem::GetRunSlotWithGoalWithoutFieldC(objHndl handle) const
{
    for (auto slotIdx = GetFirstRunSlotIdxForObj(handle);
        slotIdx != -1;
        slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {

        auto &slot = GetRunSlot(slotIdx);

		auto goal = &animationGoals_->GetByType(slot.goals[0].goalType);
        if (!goal->field_C) {
            return slot.id;
        }
    }

    return {};
}

// Originally @ 0x10055060
std::optional<AnimSlotId> AnimSystem::HasAttackAnim(objHndl handle, objHndl target) const
{
    
    AnimSlotId slotId;
    if (!DoesObjHaveGoal(handle, ag_attack, &slotId)) {
        return {};
    }

    // Check the target, if that was requested
    if (target) {

        auto slot = GetRunSlot(slotId.slotIndex);

        auto targetCheck = critterSys.GetLeaderForNpc(target);
        if (!targetCheck) {
            targetCheck = target;
        }
        auto actual = critterSys.GetLeaderForNpc(slot.goals[0].target.obj);
        if (!actual) {
            actual = slot.goals[0].target.obj;
        }

        // We have an attack goal, but not on the right target
        if (targetCheck != actual) {
            return {};
        }
    }

    return slotId;

}

// Originally @ 0x100551a0
bool AnimSystem::CurrentGoalHasField10_1(const AnimSlot &slot) const
{
    Expects(slot.IsActive());
    Expects(slot.pCurrentGoal);
    Expects(slot.pCurrentGoal && !slot.IsStackEmpty());

	auto &goal = animationGoals_->GetByType(slot.pCurrentGoal->goalType);
    
    return (goal.field_10 & 1 || (slot.pCurrentGoal->flagsData.number & 0x80));
}

// Originally @ 0x10054f30
bool AnimSystem::DoesObjHaveGoal(objHndl handle, AnimGoalType goalType, AnimSlotId * idOut) const
{
    if (!handle) {
        return false;
    }

	auto &goal = animationGoals_->GetByType(goalType);

    // Prefer direct matches, but also look for other types
    for (int i = 0; i < 4; i++) {
        // Substitute the goal type
        if (i > 0) {
            auto related = goal.relatedGoal[i - 1];
            if (related == -1) {
                break;
            }
            goalType = (AnimGoalType)related;
        }

        for (auto slotIdx = GetFirstRunSlotIdxForObj(handle);
            slotIdx != -1;
            slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {

            auto slot = mSlots[slotIdx];

            // Check both against the goal itself as well as it's "related goals"
            for (int goalIdx = 0; goalIdx < slot.currentGoal; goalIdx++) {
                if (slot.goals[goalIdx].goalType == goalType) {
                    if (idOut) {
                        *idOut = slot.id;
                    }
                    return true;
                }
            }

        }
    }

    return false;

}

// Originally @ 0x1000c500
AnimGoalPriority AnimSystem::GetCurrentPriority(objHndl handle) const
{
    for (auto slotIdx = GetFirstRunSlotIdxForObj(handle);
        slotIdx != -1;
        slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {

        auto &slot = mSlots[slotIdx];
		auto &goal = animationGoals_->GetByType(slot.goals[0].goalType);

        if (!goal.interruptAll) {
            return goal.priority;
        }
    }

    return AGP_NONE;
}

// Originally @ 0x1001AF30
void AnimSystem::SaveToMap(std::string_view mapName)
{

    auto filename = VfsPath::Concat(VfsPath::Concat("Save/Current/maps", mapName), "Anim.dat");

    bool existingData;
    void* fh;

    if (vfs->FileExists(filename)) {
        existingData = true;
        fh = vfs->Open(filename, "r+b");
    } else {
        existingData = false;
        fh = vfs->Open(filename, "wb");
    }

    if (!fh) {
        throw TempleException("Couldn't create TimeEvent data file for map!");
    }

    int slotCount = 0;
    if (existingData) {
        vfs->Seek(fh, 0);
        
        if (vfs->Read(&slotCount, sizeof(int), fh) != sizeof(int)) {
            throw TempleException("Unable to read current number of anim slots in file.");
        }
        vfs->Seek(fh, 0, SeekDir::End);
    } else if (vfs->Write(&slotCount, sizeof(slotCount), fh) != sizeof(slotCount)) {
        throw TempleException("Unable to write initial header to anim slots file");
    }

    // Save all active slots
    for (auto &slot : mSlots) {

        if (slot.IsActive()) {
            if (gameSystems->GetTeleport().IsObjectTeleporting(slot.animObj)
                && animationGoals_->GetByType(slot.goals[0].goalType).field_C) {
                SaveSlot(slot, fh);
                slotCount++;
            }
            
            InterruptGoals(slot, AGP_MAX);
        }

    }

    vfs->Seek(fh, 0);
    if (vfs->Write(&slotCount, sizeof(int), fh) != sizeof(int)) {
        throw TempleException("Failed writing out the number of slots");
    }

}

// Originally @ 0x1000c760
bool AnimSystem::ClearForObject(objHndl handle)
{
    if (gameSystems->GetLightScheme().IsUpdating()) {
        return true;
    }

    
    for (int slotIdx = GetFirstRunSlotIdxForObj(handle); slotIdx != -1; slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {
        
        auto &slot = mSlots[slotIdx];

        if (!slot.IsActive()) {
            continue;
        }

        slot.flags |= ASF_UNK11|ASF_STOP_PROCESSING;

        if (mCurrentlyProcessingSlotIdx == slotIdx) {
            continue;
        }

        // Clear the time event for this slot
        gameSystems->GetTimeEvent().Remove(TimeEventType::Anim, [=](auto &evt) {
            return evt.params[0].int32 == slot.id.slotIndex;
        });

        for (int i = slot.currentGoal; i >= 0; i--) {
            auto &goalState = slot.goals[i];
			auto &goal = animationGoals_->GetByType(goalState.goalType);
            if (goal.state_special.callback) {
                if (PrepareSlotForGoalState(slot, &goal.state_special)) {
                    goal.state_special.callback(slot);
                }
            }
        }

		FreeSlot(slot);

    }

    return true;
}

// Originally @ 0x10016C40
AnimSlot * AnimSystem::GetSlot(const AnimSlotId & id)
{
	if (id.slotIndex == -1) {
		return nullptr;
	}

	for (auto &slot : mSlots) {
		if (slot.id.slotIndex == id.slotIndex && slot.id.uniqueId == id.uniqueId) {
			return &slot;
		}
	}

	return nullptr;
}

// Originally @ 0x10055D30
// Originally @ 0x10055ED0
void AnimSystem::FreeSlot(AnimSlot & slot)
{
	if (!slot.IsActive()) {
		slot.Clear();
		return;
	}
	
	for (int i = 0; i <= slot.currentGoal; i++) {
		auto goalType = slot.goals[i].goalType;
		DecreaseActiveGoalCount(slot, animationGoals_->GetByType(goalType));
	}

	slot.Clear();
	animAddresses.slotsInUse--;

	if (!mActiveGoalCount) {
		if (mAllGoalsClearedCallback) {
			mAllGoalsClearedCallback();
		}
	}

}

// Originally @ 0x1001AC10
void AnimSystem::SaveSlot(AnimSlot & slot, void* fh) const
{

    if (vfs->Write(&slot.id.slotIndex, sizeof(int), fh) != sizeof(int)
        || vfs->Write(&slot.id.uniqueId, sizeof(int), fh) != sizeof(int)
        || vfs->Write(&slot.id.field_8, sizeof(int), fh) != sizeof(int)
        || vfs->Write(&slot.flags, sizeof(int), fh) != sizeof(int)
        || vfs->Write(&slot.currentState, sizeof(int), fh) != sizeof(int)
        || vfs->Write(&slot.field_14, sizeof(int), fh) != sizeof(int)) {
        throw TempleException("Failed to write first part of anim slot.");
    }

    if (!FrozenObjRef::Save(slot.goals[0].self.obj, &slot.goals[0].selfTracking, fh)) {
        throw TempleException("Unable to save goal object reference.");
    }

    auto currentGoalIdx = slot.currentGoal;
    if (vfs->Write(&currentGoalIdx, sizeof(int), fh) != sizeof(int)) {
        throw TempleException("Failed to write current goal count.");
    }

    // Save the individual goals
    for (int i = 0; i <= currentGoalIdx; i++) {
        SaveGoalState(slot.goals[i], fh);
    }

    static_assert(sizeof(slot.animPath) == 0xF8);
    static_assert(sizeof(GameTime) == 8);
    if (vfs->Write(&slot.animPath, sizeof(slot.animPath), fh) != sizeof(slot.animPath)
        || vfs->Write(&slot.path.occupiedFlag, 4, fh) != 4
        || vfs->Write(&slot.path.someDelay, 4, fh) != 4
        || vfs->Write(&slot.nextTriggerTime, sizeof(GameTime), fh) != 8
        || vfs->Write(&slot.gametimeSth, 8, fh) != 8
        || vfs->Write(&slot.currentPing, 4, fh) != 4) {
        throw TempleException("Failed to write the trailing animation slot state.");
    }

}

// Originally @ 0x1000c300
static void SaveGoalProperty(AnimGoalDataType dataType, const AnimParam &param, const FrozenObjRef *objRef, void *fh) {

    switch (dataType) {
    case AGDT_INT:
        if (vfs->Write(&param.number, sizeof(int), fh) != sizeof(int)) {
            throw TempleException("Failed to save integer parameter of anim goal.");
        }
        break;
    case AGDT_OBJ:
        Expects(objRef);
        if (!FrozenObjRef::Save(param.obj, objRef, fh)) {
            throw TempleException("Failed to save object parameter of anim goal.");
        }
        break;
    case AGDT_LOC:
        if (vfs->Write(&param.location, sizeof(locXY), fh) != sizeof(locXY)) {
            throw TempleException("Failed to save location parameter of anim goal.");
        }
        break;
    default:
        throw TempleException("Cannot save unknown anim goal parameter data type.");
    }
    
}

// Originally @ 0x10016cf0
void AnimSystem::SaveGoalState(const AnimSlotGoalStackEntry & goal, void * fh) const
{
    if (vfs->Write(&goal.goalType, sizeof(AnimGoalType), fh) != sizeof(AnimGoalType)) {
        throw TempleException("Failed to save goal type");
    }

    // First write the 5 obj properties that actually have tracking info
    SaveGoalProperty(AGDT_OBJ, goal.self, &goal.selfTracking, fh);
    SaveGoalProperty(AGDT_OBJ, goal.target, &goal.targetTracking, fh);
    SaveGoalProperty(AGDT_OBJ, goal.block, &goal.blockTracking, fh);
    SaveGoalProperty(AGDT_OBJ, goal.scratch, &goal.scratchTracking, fh);
    SaveGoalProperty(AGDT_OBJ, goal.parent, &goal.parentTracking, fh);

    SaveGoalProperty(AGDT_LOC, goal.targetTile, nullptr, fh);
    SaveGoalProperty(AGDT_LOC, goal.range, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.animId, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.animIdPrevious, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.animData, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.spellData, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.skillData, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.flagsData, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal1, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal2, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal3, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal4, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal5, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.scratchVal6, nullptr, fh);
    SaveGoalProperty(AGDT_INT, goal.soundHandle, nullptr, fh);

    // Write out an 8 byte 0xFF terminate. Unclear what it's used for
    uint64_t terminator = -1;
    if (vfs->Write(&terminator, sizeof(terminator), fh) != sizeof(terminator)) {
        throw TempleException("Failed to write goal terminator");
    }

}

void AnimSystem::ProcessActionCallbacks() {
    // changed to manual iteration because PerformOnAnimComplete can alter the vector
    auto initSize = mActionCallbacks.size();
    for (size_t i = 0; i < mActionCallbacks.size(); i++) {
    auto& callback = mActionCallbacks[i];
    actSeqSys.PerformOnAnimComplete(callback.obj, callback.uniqueId);
    if (initSize != mActionCallbacks.size())
    {
        int dummy = 1;
    }
    //callback.obj = 0i64;
    mActionCallbacks[i].obj = 0;
  }

  mActionCallbacks.clear();
}

void AnimSystem::PushActionCallback(AnimSlot &slot) {

  if (slot.uniqueActionId == 0) {
    return;
  }

  mActionCallbacks.push_back({slot.animObj, slot.uniqueActionId});
}

bool AnimSystem::PushGoal(AnimSlotGoalStackEntry * gdata, AnimSlotId * slotId){
    if (temple::GetRef<int>(0x10AA4BC4)) // animPrivEditorMode
        return false;

    if (config.debugMessageEnable)
        logger->debug("Goal pushed: {}", gdata->goalType);
    return PushGoalImpl(gdata, slotId, 1, 0);
}

// Originally @ 0x10056600
BOOL AnimSystem::PushGoalImpl(AnimSlotGoalStackEntry * gdata, AnimSlotId * slotId, int a3, int flags)
{
    if (!gdata) {
        logger->debug("Null goal data!");
        addresses.Debug();
    }
    if (gdata->goalType >= ag_count) {
        logger->debug("invalid goalType!");
        addresses.Debug();
    }

    auto animSysIsLoading = temple::GetRef<int>(0x10AA4BB8);
    if (animSysIsLoading)
        return FALSE;

    auto handle = gdata->self.obj;
    if (objects.getInt32(handle, obj_f_flags) & OF_DESTROYED)
        return FALSE;

    AnimSlotId & pushedId = temple::GetRef<AnimSlotId>(0x102B2648);

	auto previousSlotId = GetFirstRunSlotId(handle);
		    
    auto curGoal = 0;
    if (previousSlotId) {
		pushedId = *previousSlotId;
		auto runInfo = GetSlot(pushedId);
        if (runInfo) {
            curGoal = runInfo->currentGoal;
            if (runInfo->goals[curGoal].goalType == ag_anim_idle) {
                auto result = FALSE;
                if (runInfo->IsStackFull()) {
                    logger->debug("Anim: ERROR: Attempt to PushGoal: Goal Stack too LARGE!!! Killing the Animation Slot: AnimID: {}", pushedId.ToString());
                    logger->debug("Anim: Current SubGoal Stack is:");

                    for (auto i = 0; i < runInfo->currentGoal; i++) {
                        logger->debug("[{}]: Goal: {}", i, runInfo->goals[i].goalType);
                    }

                    runInfo->flags |= ASF_STOP_PROCESSING;
                    runInfo->currentState = 0;
                    result = FALSE;
                }
                else {
                    runInfo->currentState = 0;
                    runInfo->pCurrentGoal = &runInfo->goals[++runInfo->currentGoal];
                    *runInfo->pCurrentGoal = *gdata;

                    IncreaseActiveGoalCount(*runInfo, animationGoals_->GetByType(gdata->goalType));
                    if (slotId)
                        *slotId = pushedId;

					runInfo->pCurrentGoal->FreezeObjectRefs();

                    runInfo->uniqueActionId = 0;
                    result = TRUE;
                }
                return result;
            }
        }
    }

    if (a3) {
		auto newSlotId = AllocSlot();
        if (!newSlotId)
            return FALSE;
		pushedId = *newSlotId;
        if (slotId)
            *slotId = pushedId;
    }
    else {
        if (!slotId) {
            logger->debug("Null slot ID ptr!");
            addresses.Debug();
        }
        pushedId = *slotId;
        auto allocateThisRunIdx = temple::GetRef<BOOL(__cdecl)(AnimSlotId&)>(0x10056480);
        if (!allocateThisRunIdx(pushedId))
            return FALSE;
    }

    if (pushedId.slotIndex == -1) {
        if (slotId)
            *slotId = pushedId;
        return FALSE;
    }

    auto runInfo = &mSlots[pushedId.slotIndex];
    runInfo->currentGoal = 0;
    runInfo->currentState = 0;
    runInfo->field_14 = -1;
    runInfo->animObj = gdata->self.obj;
    runInfo->flags |= flags;
    runInfo->pCurrentGoal = &runInfo->goals[runInfo->currentGoal];
    *runInfo->pCurrentGoal = *gdata;
	runInfo->pCurrentGoal->FreezeObjectRefs();

    IncreaseActiveGoalCount(*runInfo, animationGoals_->GetByType(runInfo->pCurrentGoal->goalType));

    runInfo->uniqueActionId = 0;
    
    TimeEvent evt;
    evt.system = TimeEventType::Anim;
    evt.params[0].int32 = pushedId.slotIndex;
    evt.params[1].int32 = pushedId.uniqueId;
    evt.params[2].int32 = 3333;

    return gameSystems->GetTimeEvent().Schedule(evt, 5);

}

// Originally @ 0x10016FC0
void AnimSystem::PopGoal(AnimSlot &slot, uint32_t popFlags,
                         const AnimGoal **newGoal,
                         AnimSlotGoalStackEntry **newCurrentGoal,
                         bool *stopProcessing) {
    //logger->debug("Pop goal for {} with popFlags {:x}  (slot flags: {:x}, state {:x})", description.getDisplayName(slot.animObj), popFlags, static_cast<uint32_t>(slot.flags), slot.currentState);
  if (!slot.currentGoal && !(popFlags & ASTF_PUSH_GOAL)) {
    slot.flags |= AnimSlotFlag::ASF_STOP_PROCESSING;
  }

  if ((*newGoal)->state_special.callback) {
    if (!(popFlags & 0x70000000) || !(popFlags & 0x4000000)) {
      if (PrepareSlotForGoalState(slot, &(*newGoal)->state_special)) {
        //  logger->debug("Pop goal for {}: doing state special callback.", description.getDisplayName(slot.animObj));
        (*newGoal)->state_special.callback(slot);
      }
    }
  }

  if (!(popFlags & 0x1000000)) {
    slot.flags &= ~0x83C;
    slot.animPath.pathLength = 0; // slot->anim_path.maxPathLength = 0;
  }

  if (popFlags & ASTF_GOAL_INVALIDATE_PATH) {
    objHndl mover = slot.path.mover;
    slot.animPath.flags = 1;
    slot.path.flags = PF_NONE;
    GoalDestinationsRemove(mover);
  }

  DecreaseActiveGoalCount(slot, **newGoal);
  slot.currentGoal--;
  slot.currentState = 0;
  if (slot.IsStackEmpty()) {
    if (!(popFlags & ASTF_PUSH_GOAL)) {
      slot.flags |= AnimSlotFlag::ASF_STOP_PROCESSING;
    //  logger->debug("Pop goal for {}: stopping processing (last goal was {}).", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType]);
    }
  } else {
    auto prevGoal = &slot.goals[slot.currentGoal];
    //logger->debug("Popped goal {}, new goal is {}", animGoalTypeNames[slot.pCurrentGoal->goalType], animGoalTypeNames[prevGoal->goalType]);
    slot.pCurrentGoal = *newCurrentGoal = &slot.goals[slot.currentGoal];
	*newGoal = &animationGoals_->GetByType((*newCurrentGoal)->goalType);
    *stopProcessing = false;
    if (prevGoal->goalType == ag_anim_fidget) {
        int dummy = 1;
      // FIX: prevents ag_anim_fidget from queueing an AnimComplete call (which
      // creates the phantom animId = 0 bullshit)
    } else if ((*newCurrentGoal)->goalType == ag_anim_idle &&
               !(popFlags & ASTF_PUSH_GOAL)) {
      PushActionCallback(slot);
    }
  }
 // logger->debug("PopGoal: Ending with slot flags {:x}, state {:x}", static_cast<uint32_t>(slot.flags), slot.currentState);
}

/*
When an event should be re-executed at a later time, but unmodified, this
method is used. It also checks whether animations should "catch up" (by skipping
frames essentially), or whether they should be run at whatever speed was
intended,
but visibly slowing down.
*/
BOOL AnimSystem::RescheduleEvent(int delayMs, AnimSlot &slot,
                                 const TimeEvent *oldEvt) {
  TimeEvent evt;
  evt.system = TimeEventType::Anim;
  evt.params[0].int32 = slot.id.slotIndex;
  evt.params[1].int32 = slot.id.uniqueId;
  evt.params[2].int32 =
      1111; // Some way to identify these rescheduled events???

  if (config.animCatchup) {
    return gameSystems->GetTimeEvent().ScheduleAbsolute(evt, oldEvt->time, delayMs,
                                                 &slot.nextTriggerTime);
  } else {
    return gameSystems->GetTimeEvent().Schedule(evt, delayMs, &slot.nextTriggerTime);
  }
}

void AnimSystem::GoalDestinationsRemove(objHndl obj) {
  static auto goal_destinations_remove =
      temple::GetPointer<void(objHndl)>(0x100bac80);
  goal_destinations_remove(obj);
}

bool AnimSystem::Interrupt(objHndl actor, AnimGoalPriority priority, bool all)
{
    auto lastSlot = -1;
    if (priority < AGP_NONE || priority >= AGP_MAX) {
        logger->debug("Invalid priority provided: {}", (int)priority);
        addresses.Debug();
    }

    if (all) {
        priority = AGP_NONE;
    }

    auto slotIdx = GetFirstRunSlotIdxForObj(actor);
    if (slotIdx == -1)
        return true;

    while (slotIdx != lastSlot) {
        lastSlot = slotIdx;
        if (slotIdx != -1 && !InterruptGoals(mSlots[slotIdx], priority)) {
            return false;
        }
        slotIdx = GetNextRunSlotIdxForObj(actor, slotIdx); // FindNextSlotIdx
        if (slotIdx == -1)
            return true;
    }
    return false;
}

// Originally @ 0x10056090
bool AnimSystem::InterruptGoals(AnimSlot &slot, AnimGoalPriority priority) {


  assert(slot.id.slotIndex < 512);

  if (!(slot.flags & ASF_ACTIVE)) {
      return false;
  }

  if (!gameSystems->IsResetting() && slot.currentGoal != -1) {
    auto &stackTop = slot.goals[slot.currentGoal];
	auto &goal = animationGoals_->GetByType(stackTop.goalType);

    if (priority < AnimGoalPriority::AGP_HIGHEST){
        if (goal.interruptAll) {
            return true;
        }
        if (goal.priority == AnimGoalPriority::AGP_5) {
            return false;
        }
    }
    

    if (goal.priority == AnimGoalPriority::AGP_3) {
      if (priority < AnimGoalPriority::AGP_3) {
        return false;
      }
    } else if (goal.priority == AnimGoalPriority::AGP_2) {
      if (priority < 2) {
        return false;
      }
    } else if (goal.priority >= priority) {
      if (goal.priority != AnimGoalPriority::AGP_7) {
        return false;
      }
      slot.flags &= ~ASF_UNK5;
    }
  }

  auto goalType = &animationGoals_->GetByType(slot.goals[0].goalType);
  if (goalType->priority >= AnimGoalPriority::AGP_7 &&
      priority < AnimGoalPriority::AGP_7) {
    auto pNewStackTopOut = &slot.goals[slot.currentGoal];
    for (goalType = &animationGoals_->GetByType(pNewStackTopOut->goalType);
         goalType->priority < AnimGoalPriority::AGP_7;
         goalType = &animationGoals_->GetByType(pNewStackTopOut->goalType)) {
      bool stopProcessing = false;
      PopGoal(slot, 0x30000000, &goalType, &pNewStackTopOut, &stopProcessing);
      pNewStackTopOut = &slot.goals[slot.currentGoal];
    }
    return true;
  }

  slot.flags |= ASF_STOP_PROCESSING;

  if (mCurrentlyProcessingSlotIdx == slot.id.slotIndex) {
    return true;
  }

  // Removes all time events for the slot
  gameSystems->GetTimeEvent().Remove(
      TimeEventType::Anim, [&](const TimeEvent &evt) {
        return evt.params[0].int32 == slot.id.slotIndex;
      });

  if (slot.currentGoal != -1) {
    if (!slot.pCurrentGoal) {
      slot.pCurrentGoal = &slot.goals[slot.currentGoal];
    }
    for (auto i = slot.currentGoal; i >= 0; i--) {
	  auto &goal = animationGoals_->GetByType(slot.goals[i].goalType);
      if (!gameSystems->IsResetting()) {
        if (goal.state_special.callback) {
          if (PrepareSlotForGoalState(slot, &goal.state_special)) {
            goal.state_special.callback(slot);
          }
        }
      }
    }
  }

  FreeSlot(slot);

  return false;
}

// Originally @ 0x10056350
void AnimSystem::InterruptGoalsByType(objHndl handle, AnimGoalType type, AnimGoalType keep)
{
	// type is always ag_flee, keep is always -1 where we call it

	AnimGoalPriority interruptPriority;

	if (keep == -1) {
		interruptPriority = AGP_HIGHEST;
	} else {
		auto &goal = animationGoals_->GetByType(keep);
		interruptPriority = goal.priority;
		Expects(interruptPriority >= AGP_NONE && interruptPriority <= AGP_HIGHEST);
		if (goal.interruptAll) {
			interruptPriority = AGP_NONE;
		}
	}

	for (int slotIdx = GetFirstRunSlotIdxForObj(handle); slotIdx != -1; slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {
		auto &slot = GetRunSlot(slotIdx);
		if (slot.goals[0].goalType == type || slot.pCurrentGoal->goalType == type) {
			InterruptGoals(slot, interruptPriority);
		}
	}
}

// Originally @ 0x1000c8d0
void AnimSystem::InterruptAllGoalsUpToPriority(AnimGoalPriority priority)
{
	Expects(priority >= AGP_NONE && priority <= AGP_HIGHEST);
	for (auto &slot : mSlots) {
		if (slot.IsActive()) {
			if (!InterruptGoals(slot, priority)) {
				logger->error("Failed to interrupt anim goals of priority {} on slot {}",
					priority, slot.id.slotIndex);
			}
		}
	}
}

// Originally @ 0x10056a50
bool AnimSystem::AddSubGoal(const AnimSlotId & id, AnimSlotGoalStackEntry & stackEntry)
{
	Expects(stackEntry.goalType >= 0 && stackEntry.goalType < ag_count);

	if (id.slotIndex == -1) {
		return PushGoalImpl(&stackEntry, nullptr, 0, 0);
	}

	auto slot = GetSlot(id);
	if (!slot) {
		logger->error("Cannot add subgoal to invalid animation slot {}", id);
		return false;
	}

	if (slot->IsStackFull()) {
		return false;
	}

	Expects(!slot->IsStackEmpty());

	// Since this is "prepending" to the stack
	// We have to move all stack entries backwards
	if (++slot->currentGoal > 0) {
		for (int i = slot->currentGoal; i >= 1; i--) {
			slot->goals[i] = slot->goals[i - 1];
		}
	}
	slot->pCurrentGoal = &slot->goals[slot->currentGoal];

	slot->goals[0] = stackEntry;
	slot->goals[0].FreezeObjectRefs();

	if (slot->field_14 != -1) {
		++slot->field_14;
	}

	IncreaseActiveGoalCount(*slot, animationGoals_->GetByType(stackEntry.goalType));
	
	return true;
}

bool AnimSystem::PrepareSlotForGoalState(AnimSlot &slot,
                                         const AnimGoalState *state) {
  static auto anim_prepare_run_for_goalstate =
      temple::GetPointer<int(AnimSlot * runSlot, const AnimGoalState *)>(
          0x10055700);
  return anim_prepare_run_for_goalstate(&slot, state) == TRUE;
}

bool AnimSystem::IsEquivalentGoalType(AnimGoalType expected, AnimGoalType actual) const {
    if (expected == actual) {
        return true;
    }

	auto &goal = animationGoals_->GetByType(expected);
    for (int i = 0; i < 3; i++) {
        if (goal.relatedGoal[i] == -1) {
            break;
        }
        if (goal.relatedGoal[i] == actual) {
            return true;
        }
    }

    return false;
}

std::optional<AnimSlotId> AnimSystem::GetSlotForGoalAndObjs(objHndl handle, const AnimSlotGoalStackEntry & goalData) const
{

	// Iterate over all slots belonging to the object
    for (int slotIdx = GetFirstRunSlotIdxForObj(handle); slotIdx != -1; slotIdx = GetNextRunSlotIdxForObj(handle, slotIdx)) {
        
        auto &slot = mSlots[slotIdx];
        auto &firstGoalState = slot.goals[0];
        if (!IsEquivalentGoalType(goalData.goalType, firstGoalState.goalType)) {
            continue;
        }

        if (firstGoalState.self.obj == goalData.self.obj
            && firstGoalState.target.obj == goalData.target.obj
            && firstGoalState.block.obj == goalData.block.obj
            && firstGoalState.scratch.obj == goalData.scratch.obj
            && firstGoalState.parent.obj == goalData.parent.obj) {
            return slot.id;
        }
    }

    return {};
}

void AnimSystem::DebugGoals()
{
    logger->debug("Currently Existing Animations");
    logger->debug("------------------------------------------------");
    for (auto i = 0; i < ANIM_RUN_SLOT_CAP; i++) {
        auto &slot = mSlots[i];
        if (slot.flags){
            logger->debug("In slot {}", i);
        }
    }
    logger->debug("------------------------------------------------");
}

// Originally @ 0x10055BF0
void AnimSystem::IncreaseActiveGoalCount(const AnimSlot & slot, const AnimGoal & goal)
{
	if (goal.priority >= AGP_2 && !goal.interruptAll && !CurrentGoalHasField10_1(slot))
	{
		Expects(mActiveGoalCount >= 0);
		++mActiveGoalCount;
	}
}

// Originally @ 0x10055ca0
void AnimSystem::DecreaseActiveGoalCount(const AnimSlot & slot, const AnimGoal & goal)
{
	if (goal.priority >= AGP_2 && !goal.interruptAll && !CurrentGoalHasField10_1(slot) && mActiveGoalCount >= 1) {
		--mActiveGoalCount;
	}
}

std::string AnimSlotId::ToString() const {
  return format("[{}:{}r{}]", slotIndex, uniqueId, field_8);
}

int GoalStateFuncs::GoalStateFunc106(AnimSlot &slot) {
    //logger->debug("GSF 106 for {}, goal {}, flags {:x}, currentState {:x}", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], (uint32_t)slot.flags, (uint32_t)slot.currentState);
  auto obj = slot.param1.obj;
  assert(slot.param1.obj);

  auto aasHandle = objects.GetAnimHandle(obj);
  assert(aasHandle);

  if (objects.getInt32(obj, obj_f_spell_flags) & SpellFlags::SF_10000) {
      //logger->debug("GSF 106: return FALSE due to obj_f_spell_flags 0x10000");
    return FALSE;
  }

  auto animId = aasHandle->GetAnimId();

  if ((!objects.IsCritter(obj) ||
       !(objects.getInt32(obj, obj_f_critter_flags) &
         (OCF_PARALYZED | OCF_STUNNED)) ||
       !animId.IsWeaponAnim() &&
           (animId.GetNormalAnimType() == gfx::NormalAnimType::Death ||
            animId.GetNormalAnimType() == gfx::NormalAnimType::Death2 ||
            animId.GetNormalAnimType() == gfx::NormalAnimType::Death3))) {
    static auto anim_frame_advance_maybe =
        temple::GetPointer<BOOL(objHndl , AnimSlot & runSlot,
                                uint32_t animHandle, uint32_t * eventOut)>(
            0x10016530);
    uint32_t eventOut = 0;
    anim_frame_advance_maybe(obj, slot, aasHandle->GetHandle(), &eventOut);

    // This is the ACTION trigger
    if (eventOut & 1) {
      slot.flags |= AnimSlotFlag::ASF_UNK3;
      //logger->debug("GSF 106: Set flag 4, returned TRUE");
      return TRUE;
    }

    // If the animation is a looping animation, it does NOT have a
    // defined end, so we just trigger the end here anyway otherwise
    // this'll loop endlessly
    bool looping = false;
    /*if (animId.IsWeaponAnim() && ( animId.GetWeaponAnim() == gfx::WeaponAnim::Idle || animId.GetWeaponAnim() == gfx::WeaponAnim::CombatIdle)) {*/

    

    if (animId.IsWeaponAnim() && (animId.GetWeaponAnim() == gfx::WeaponAnim::Idle )) {
    //	// We will continue anyway down below, because the character is idling, so log a message
        if (!(eventOut & 2)) {
            logger->info("Ending wait for animation action/end in goal {}, because the idle animation would never end.",
                slot.pCurrentGoal->goalType);
        }
        looping = true;
    }

    // This is the END trigger
    if (!looping && !(eventOut & 2))
    {
        //logger->debug("GSF 106: eventOut & 2, returned TRUE");
        return TRUE;
    }
      

    // Clears WaypointDelay flag
    auto gameObj = objSystem->GetObject(obj);
    if (objects.IsNPC(obj)) {
      auto flags = gameObj->GetInt64(obj_f_npc_ai_flags64);
      gameObj->SetInt64(obj_f_npc_ai_flags64, flags & 0xFFFFFFFFFFFFFFFDui64);
    }

    // Clear 0x10 slot flag
    slot.flags &= ~AnimSlotFlag::ASF_UNK5;
  }
  //logger->debug("GSF 106: returned FALSE");
  return FALSE;
}

int GoalStateFuncs::GoalStateFunc130(AnimSlot& slot)
{
    //logger->debug("GSF 130");
    uint32_t eventOut = 0;
    auto handle = slot.param1.obj;
    if (!handle)
    {
        logger->warn("Missing anim object!");
        return FALSE;
    }

    auto obj = gameSystems->GetObj().GetObject(handle);
    auto aasHandle = objects.GetAnimHandle(handle);
    if (!aasHandle || !aasHandle->GetHandle())
    {
        logger->warn("No aas handle!");
        return FALSE;
    }

    if (obj->GetInt32(obj_f_spell_flags) & SpellFlags::SF_10000)
        return FALSE;
    
    if (obj->IsCritter())
    {
        if (obj->GetInt32(obj_f_critter_flags) & (OCF_PARALYZED | OCF_STUNNED))
            return FALSE;
    }
    animAddresses.anim_frame_advance_maybe(handle, slot, aasHandle->GetHandle(), &eventOut);

    if (eventOut & 1)
        slot.flags |= AnimSlotFlag::ASF_UNK3;

    if (eventOut & 2) {
        slot.flags &= ~(AnimSlotFlag::ASF_UNK5);
        return FALSE;
    }
    return TRUE;
}

int GoalStateFuncs::GoalStateFuncPickpocket(AnimSlot & slot)
{

    auto tgt = slot.param2.obj;
    auto handle = slot.param1.obj;
    int gotCaught = 0;

    
    if ( !tgt 
        || (objects.GetFlags(handle) & (OF_OFF | OF_DESTROYED))
        || (objects.GetFlags(tgt) & (OF_OFF | OF_DESTROYED)) )
        return FALSE;

    critterSys.Pickpocket(handle, tgt, gotCaught);
    if (!gotCaught){
        slot.flags |= AnimSlotFlag::ASF_UNK12;
    }
    return TRUE;
}



int GoalStateFuncs::GoalStateFunc83(AnimSlot &slot) {
  //logger->debug("GSF83 for {}, current goal {} ({})", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal);
  auto flags = slot.flags;
  if (flags & AnimSlotFlag::ASF_UNK3 && !(flags & AnimSlotFlag::ASF_UNK4)) {
    slot.flags = flags | AnimSlotFlag::ASF_UNK4;
    //logger->debug("GSF83 return TRUE");
    return TRUE;
  } else {
    return FALSE;
  }
}

static BOOL goalstatefunc_45(AnimSlot &slot) {
    //logger->debug("GSF45");
  auto obj = slot.param1.obj;
  assert(obj);
  auto aasHandle = objects.GetAnimHandle(obj);
  assert(aasHandle);

  auto animId = aasHandle->GetAnimId();
  return animId.IsConjuireAnimation() ? TRUE : FALSE;
}

static BOOL goalstatefunc_41(AnimSlot &slot) {
    //logger->debug("GSF41");
  auto spell = spellsCastRegistry.get(slot.param1.number);
  return FALSE;
}

static BOOL goalstatefunc_42(AnimSlot &slot) {
    //logger->debug("GSF42");
  auto obj = slot.param1.obj;
  assert(obj);

  auto spellId = slot.param2.number;
  slot.flags |= 0xCu; // Sets 8 and 4

  if (spellId) {
    actSeqSys.ActionFrameProcess(obj);
    pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);

    auto spell = spellsCastRegistry.get(spellId);
    if (spell) {
      auto targetCount = spell->spellPktBody.targetCount;
      bool found = false;
      for (uint32_t i = 0; i < targetCount; i++) {
        if (spell->spellPktBody.targetListHandles[i] ==
            spell->spellPktBody.caster) {
          found = true;
          break;
        }
      }

      if (found) {
        DispIOTurnBasedStatus dispIo;
        dispIo.tbStatus = actSeqSys.curSeqGetTurnBasedStatus();
        dispatch.dispatchTurnBasedStatusInit(spell->spellPktBody.caster,
                                             &dispIo);
      }
    }
  }

  return TRUE;
}

static class AnimSystemHooks : public TempleFix {
public:
	
  static void RasterPoint(int64_t x, int64_t y, LineRasterPacket & s300);
  static int RasterizeLineBetweenLocs(locXY loc, locXY tgtLoc, int8_t *deltas);
  static void RasterizeLineScreenspace(int64_t x, int64_t y, int64_t tgtX, int64_t tgtY, LineRasterPacket &s300, void(__cdecl*callback)(int64_t, int64_t, LineRasterPacket &));

  static BOOL TargetDistChecker(objHndl handle, objHndl tgt);

  static float HookedGetRunSpeed(objHndl handle, obj_f field);

  void apply() override {


      // Animation pathing stuff
      replaceFunction(0x1003DF30, RasterPoint);

      replaceFunction(0x1003FB40, RasterizeLineBetweenLocs);

      static BOOL(__cdecl *orgGoalstate_20)(AnimSlot&) = replaceFunction<BOOL(__cdecl)(AnimSlot &)>(0x1000EC10, [](AnimSlot &runSlot)  {
          return orgGoalstate_20(runSlot);
      });
      //static bool useNew = false;


      replaceFunction(0x10017BF0, TargetDistChecker);

    // goalstatefunc_133
    replaceFunction<BOOL(AnimSlot &)>(0x1001C100, gsFuncs.GoalStateFunc133);
    // goalstatefunc_106
    replaceFunction<BOOL(AnimSlot &)>(0x100185e0, gsFuncs.GoalStateFunc106);
    // goalstatefunc_100
    replaceFunction<BOOL(AnimSlot &)>(0x1000D150, gsFuncs.GoalStateFunc100_IsCurrentPathValid);
    // goalstatefunc_83_checks_flag4_set_flag8
    replaceFunction<BOOL(AnimSlot &)>(0x10012c80, gsFuncs.GoalStateFunc83);
    // goalstatefunc_82
    replaceFunction<BOOL(AnimSlot &)>(0x10012C70, gsFuncs.GoalStateFunc82);
    //goalstatefunc_78_isheadingok
    replaceFunction<BOOL(AnimSlot &)>(0x100125F0, gsFuncs.GoalStateFunc78_IsHeadingOk);
    //goalstatefunc_70
//	replaceFunction<BOOL(AnimSlot &)>(0x10011D40, gsFuncs.GoalStateFunc70);  // not sure this is correct, not needed anyway right now
    // goalstatefunc_65
    replaceFunction<BOOL(AnimSlot &)>(0x10011880, gsFuncs.GoalStateFunc65);
    // gsf54
    replaceFunction<BOOL(AnimSlot &)>(0x10010D60, gsFuncs.GoalStateFunc54);
    // goalstatefunc_48
    replaceFunction<BOOL(AnimSlot &)>(0x100107E0, gsFuncs.GoalStateFuncPickpocket);
    // goalstatefunc_45
    replaceFunction<BOOL(AnimSlot &)>(0x10010520, goalstatefunc_45);
    // goalstatefunc_41
    replaceFunction<BOOL(AnimSlot &)>(0x10010290, goalstatefunc_41);
    // goalstatefunc_42
    replaceFunction<BOOL(AnimSlot &)>(0x100102c0, goalstatefunc_42);
    //goalstatefunc_35
    replaceFunction<BOOL(AnimSlot&)>(0x1000FF10, gsFuncs.GoalStateFunc35);
    //goalstate is concealed
    replaceFunction<BOOL(AnimSlot&)>(0x1000E250, gsFuncs.GoalStateFuncIsParam1Concealed);
    // goal_is_critter_prone
    replaceFunction<BOOL(AnimSlot&)>(0x1000E270, gsFuncs.GoalStateFuncIsCritterProne);


    writeCall(0x100148EA, HookedGetRunSpeed);

  }
} animHooks;

static string getDelayText(AnimStateTransition trans) {
  string delay = "";
  if (trans.delay == AnimStateTransition::DelayRandom) {
    delay = ", delay: random";
  } else if (trans.delay == AnimStateTransition::DelayCustom) {
    delay = ", delay: custom";
  } else if (trans.delay == AnimStateTransition::DelaySlot) {
    delay = ", delay: slot";
  } else if (trans.delay != 0) {
    delay = format(", delay: {}", trans.delay);
  }
  return delay;
}

static void getTransitionText(string &diagramText, int &j,
                              const AnimStateTransition &transition,
                              const char *condition) {
  string delay = getDelayText(transition);
  auto newState = transition.newState;
  if (newState & 0xFF000000) {
    logger->info("New state flags {:x}", newState);
    if ((newState & 0x30000000) == 0x30000000) {
      diagramText += format("state{} --> [*] : [{}{}]\n", j, condition, delay);
    } else if ((newState & TRANSITION_GOAL) == TRANSITION_GOAL) {
      auto newGoal = newState & 0xFFF;
      diagramText += format("state{} --> [*] : [{}{}] to {}\n", j, condition,
                            delay, (AnimGoalType) newGoal);
    } else if ((newState & TRANSITION_UNK1) == TRANSITION_UNK1) {
      diagramText += format("state{} --> [*] : [{}{}, flags: 0x90]\n", j,
                            condition, delay);
    } else if (newState & TRANSITION_LOOP) {
      diagramText +=
          format("state{} --> state0 : [{}{}, reset]\n", j, condition, delay);
    } else {
      diagramText += format("state{} --> state0 : [{}{}, flags: {}]\n", j,
                            condition, delay, newState);
    }
  } else {
    // Normal transition
    diagramText += format("state{} --> state{} : [{}{}]\n", j, newState - 1,
                          condition, delay);
  }
}

std::string GetAnimParamName(int animParamType) {
  if (animParamType < 21) {
    return AnimGoalDataNames[animParamType];
  } else if (animParamType == 31) {
    return "SELF_OBJ_PRECISE_LOC";
  } else if (animParamType == 32) {
    return "TARGET_OBJ_PRECISE_LOC";
  } else if (animParamType == 33) {
    return "NULL_HANDLE";
  } else if (animParamType == 34) {
    return "TARGET_LOC_PRECISE";
  } else {
    return to_string(animParamType);
  }
}

static json11::Json::object
TransitionToJson(const AnimStateTransition &transition) {
  using namespace json11;
  Json::object result;

  if (transition.delay == 0) {
    result["delay"] = nullptr;
  } else if (transition.delay == AnimStateTransition::DelayCustom) {
    result["delay"] = "custom";
  } else if (transition.delay == AnimStateTransition::DelayRandom) {
    result["delay"] = "random";
  } else if (transition.delay == AnimStateTransition::DelaySlot) {
    result["delay"] = "slot";
  } else {
    result["delay"] = transition.delay;
  }

  result["newState"] = (int)(transition.newState & 0xFFFFFF);

  auto flags = (int)((transition.newState >> 24) & 0xFF);
  result["flags"] = flags;

  return result;
}

static json11::Json::object StateToJson(const AnimGoalState &state,
                                        map<uint32_t, string> &goalFuncNames,
                                        map<uint32_t, string> &goalFuncDescs) {
  using namespace json11;
  Json::object result{
      {"callback", fmt::format("0x{:x}", (uint32_t)state.callback)},
      {"name", goalFuncNames[(uint32_t)state.callback]},
      {"description", goalFuncDescs[(uint32_t)state.callback]},
      {"refToOtherGoalType", state.flagsData}};

  if (state.afterSuccess.newState == state.afterFailure.newState &&
      state.afterSuccess.delay == state.afterFailure.delay) {
    result["transition"] = TransitionToJson(state.afterSuccess);
  } else {
    result["trueTransition"] = TransitionToJson(state.afterSuccess);
    result["falseTransition"] = TransitionToJson(state.afterFailure);
  }

  if (state.argInfo1 == -1) {
    result["arg1"] = Json();
  } else {
    result["arg1"] = GetAnimParamName(state.argInfo1);
  }

  if (state.argInfo2 == -1) {
    result["arg2"] = Json();
  } else {
    result["arg2"] = GetAnimParamName(state.argInfo2);
  }

  return result;
}

int GoalStateFuncs::GoalStateFunc35(AnimSlot& slot)
{
    //logger->debug("GoalStateFunc35");
    if (slot.param1.obj)
    {
        if (!gameSystems->GetObj().GetObject(slot.param1.obj)->IsCritter()
            || !critterSys.IsDeadNullDestroyed(slot.param1.obj))
            return 1;
    }

    return 0;
}

int GoalStateFuncs::GoalStateFunc54(AnimSlot& slot){
    //logger->debug("GSF54 ag_attempt_attack action frame");
    assert(slot.param1.obj);
    assert(slot.param2.obj);

    if (! (gameSystems->GetObj().GetObject(slot.param2.obj)->GetFlags() & (OF_OFF | OF_DESTROYED)) )
    {
        actSeqSys.ActionFrameProcess(slot.param1.obj);
    }
    return TRUE;
}

int GoalStateFuncs::GoalStateFunc133(AnimSlot& slot)
{
    // 1001C100
    //logger->debug("GoalStateFunc133");
    auto sub_10017BF0 = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10017BF0);
    auto result = sub_10017BF0(slot.param1.obj, slot.param2.obj);
    if (!result)
    {
        if (combatSys.isCombatActive())
        {
            logger->debug("Anim sys for {} ending turn...", description.getDisplayName(slot.param1.obj));
            combatSys.CombatAdvanceTurn(slot.param1.obj);
        }
    }

    return result;
}

int GoalStateFuncs::GoalStateFuncIsCritterProne(AnimSlot& slot)
{
    // 1000E270
    //logger->debug("GoalStateFunc IsCritterProne");
    if (slot.param1.obj){
        return objects.IsCritterProne(slot.param1.obj);
    }

    logger->debug("Anim Assertion failed: obj != OBJ_HANDLE_NULL");
    return FALSE;
}

int GoalStateFuncs::GoalStateFuncIsParam1Concealed(AnimSlot& slot)
{
    //logger->debug("GoalState IsConcealed");
    return (critterSys.IsConcealed(slot.param1.obj));
}

int GoalStateFuncs::GoalStateFunc78_IsHeadingOk(AnimSlot & slot){
    if (!slot.pCurrentGoal) {
        slot.pCurrentGoal = &slot.goals[slot.currentGoal];
    }

    if (slot.path.nodeCount <= 0)
        return 1;

    // get the mover's location
    auto obj = gameSystems->GetObj().GetObject(slot.param1.obj);
    auto objLoc = obj->GetLocationFull();
    float objAbsX, objAbsY;
    locSys.GetOverallOffset(objLoc, &objAbsX, &objAbsY);

    // get node loc
    if (slot.path.currentNode > 200 || slot.path.currentNode < 0) {
        logger->info("Anim: Illegal current node detected!");
        return 1;
    }

    auto nodeLoc = slot.path.nodes[slot.path.currentNode];
    float nodeAbsX, nodeAbsY;
    locSys.GetOverallOffset(nodeLoc, &nodeAbsX, &nodeAbsY);

    if (objAbsX == nodeAbsX && objAbsY == nodeAbsY) {
        if (slot.path.currentNode + 1 >= slot.path.nodeCount)
            return 1;
        nodeLoc = slot.path.nodes[slot.path.currentNode + 1];
        locSys.GetOverallOffset(nodeLoc, &nodeAbsX, &nodeAbsY);
    }

    auto &rot = slot.pCurrentGoal->scratchVal2.floatNum;
    rot = (float)(M_PI_2 + M_PI * 0.75 - atan2(nodeAbsY - objAbsY, nodeAbsX - objAbsX) );
    if (rot < 0.0) {
        rot += (float)6.2831853;//M_PI * 2;
    }
    if (rot > 6.2831853) {
        rot -= (float) 6.2831853;//M_PI * 2;
    }

    
    auto objRot = obj->GetFloat(obj_f_rotation);


    if (sin(objRot - rot) > 0.017453292)    // 1 degree
        return 0;
    
    if (cos(objRot) - cos(rot) > 0.017453292) // in case it's a 180 degrees difference
        return 0;

    return 1;
}

int GoalStateFuncs::GoalStateFunc82(AnimSlot& slot)
{ //10012C70
    /*if (slot.pCurrentGoal && slot.pCurrentGoal->goalType != ag_anim_idle) {
        logger->debug("GSF82 for {}, current goal {} ({}). Flags: {:x}, currentState: {:x}", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal, slot.flags, slot.currentState);
        if(slot.pCurrentGoal->goalType == ag_hit_by_weapon)
        {
            int u = 1;
        }
    }*/
    //return (slot.flags & AnimSlotFlag::ASF_UNK5) == 0? TRUE: FALSE;
    return (~(slot.flags >> 4)) & 1;
    //return ~(slot.flags >> 4) & 1;
}

int GoalStateFuncs::GoalStateFunc65(AnimSlot& slot)
{
    //logger->debug("GSF65");
    if (!slot.param1.obj)
    {
        logger->warn("Error in GSF65");
        return FALSE;
    }
    auto obj = gameSystems->GetObj().GetObject(slot.param1.obj);
    auto locFull = obj->GetLocationFull();
    float worldX, worldY;
    locSys.GetOverallOffset(locFull, &worldX, &worldY);
    
    auto obj2 = gameSystems->GetObj().GetObject(slot.param2.obj);
    auto loc2 = obj2->GetLocationFull();
    float worldX2, worldY2;
    locSys.GetOverallOffset(loc2, &worldX2, &worldY2);

    auto rot = obj->GetFloat(obj_f_rotation);

    auto newRot = atan2(worldY2 - worldY, worldX2 - worldX) + M_PI * 3 / 4 - rot;
    while (newRot > M_PI * 2) newRot -= M_PI * 2;
    while (newRot < 0) newRot += M_PI * 2;

    auto newRotAdj = newRot - M_PI / 4;

    gfx::WeaponAnim weaponIdParam = gfx::WeaponAnim::FrontHit;
    if (newRotAdj < M_PI / 4)
        weaponIdParam = gfx::WeaponAnim::FrontHit;
    else if (newRotAdj < M_PI*3/4)
        weaponIdParam = gfx::WeaponAnim::LeftHit;
    else if (newRotAdj < M_PI * 5 / 4)
        weaponIdParam = gfx::WeaponAnim::BackHit;
    else if (newRotAdj < M_PI * 7 / 4)
        weaponIdParam = gfx::WeaponAnim::RightHit;

    auto weaponAnimId = critterSys.GetAnimId(slot.param1.obj, weaponIdParam);
    objects.SetAnimId(slot.param1.obj, weaponAnimId);

    return TRUE;

}

int GoalStateFuncs::GoalStateFunc70(AnimSlot & slot)
{
    auto someGoal = slot.stateFlagData;

    if (someGoal == -1){
        SpellPacketBody spPkt;
        spellSys.GetSpellPacketBody(slot.param1.number, &spPkt);
        if (spPkt.animFlags & 8){
            slot.pCurrentGoal->animId.number = spPkt.spellRange; // weird shit!
            return TRUE;
        }
        return FALSE;
    } 
    else{
        slot.pCurrentGoal->animId.number = someGoal;
        return TRUE;
    }
}

int GoalStateFuncs::GoalStateFunc100_IsCurrentPathValid(AnimSlot & slot)
{
    return slot.path.flags & PathFlags::PF_COMPLETE;
}

void AnimSystemHooks::RasterPoint(int64_t x, int64_t y, LineRasterPacket & rast)
{
    auto someIdx = rast.deltaIdx;
    if (someIdx == -1 || someIdx >= 200) {
        rast.deltaIdx = -1;
        return;
    }

    rast.counter++;

    if (rast.counter == rast.interval) {
        rast.deltaXY[someIdx] = x - rast.x;
        rast.deltaXY[rast.deltaIdx + 1] = y - rast.y;
        rast.x = x;
        rast.y = y;
        rast.deltaIdx += 2;
        rast.counter = 0;
    }
}

int AnimSystemHooks::RasterizeLineBetweenLocs(locXY loc, locXY tgtLoc, int8_t * deltas){
    // implementation of the Bresenham line algorithm
    LineRasterPacket rast;
    int64_t locTransX, locTransY, tgtTransX, tgtTransY;
    auto getTranslation = temple::GetRef<void(__cdecl)(locXY, int64_t &, int64_t &)>(0x10028E10);
    getTranslation(loc, locTransX, locTransY);
    locTransX += 20;
    locTransY += 14;
    getTranslation(tgtLoc, tgtTransX, tgtTransY);
    tgtTransX += 20;
    tgtTransY += 14;

    rast.x = locTransX;
    rast.y = locTransY;
    rast.deltaXY = deltas;

    animHooks.RasterizeLineScreenspace(locTransX, locTransY, tgtTransX, tgtTransY, rast, animHooks.RasterPoint);

    if (rast.deltaIdx == -1)
        return 0;

    while (rast.counter){
        animHooks.RasterPoint(tgtTransX, tgtTransY, rast);
    }

    return rast.deltaIdx == -1 ? 0 : rast.deltaIdx;

}

void AnimSystemHooks::RasterizeLineScreenspace(int64_t x0, int64_t y0, int64_t tgtX, int64_t tgtY, LineRasterPacket & s300, void(*callback)(int64_t, int64_t, LineRasterPacket &)){
    auto x = x0, y = y0;
    auto deltaX = tgtX - x0, deltaY = tgtY - y0;
    auto deltaXAbs = abs(deltaX), deltaYAbs = abs(deltaY);


    auto extentX = 2 * deltaXAbs, extentY = 2 * deltaYAbs;

    auto deltaXSign = 0, deltaYSign = 0;
    if (deltaX > 0)
        deltaXSign = 1;
    else if (deltaX < 0)
        deltaXSign = -1;

    if (deltaY > 0)
        deltaYSign = 1;
    else if (deltaY < 0)
        deltaYSign = -1;

    
    if (extentX <= extentY){

        int64_t D = extentX - (extentY / 2);
        callback(x0, y0, s300);
        while (y != tgtY){
            if (D >= 0){
                x += deltaXSign;
                D -= extentY;
            }
            D += extentX;
            y += deltaYSign;
            callback(x, y, s300);
        }
    } 
    else
    {
        int64_t D = extentY - (extentX / 2);
        callback(x0, y0, s300);
        while (x != tgtX){
            
            if (D >= 0){
                y += deltaYSign;
                D -= extentX;
            }
            D += extentY;
            x += deltaXSign;
            callback(x, y, s300);
        }
    }
}

BOOL AnimSystemHooks::TargetDistChecker(objHndl handle, objHndl tgt){

    if (!handle || !objects.IsCritter(handle) || critterSys.IsDeadOrUnconscious(handle)
        || !tgt || !objects.IsCritter(tgt) || critterSys.IsDeadNullDestroyed(tgt))
        return FALSE;

    if ((objects.getInt32(handle, obj_f_spell_flags) & SpellFlags::SF_4)
        && !(objects.getInt32(handle, obj_f_critter_flags2) & CritterFlags2::OCF2_ELEMENTAL))
    {
        if (config.debugMessageEnable)
            logger->debug("TargetDistChecker: spell flag 4 error");
        return FALSE;
    }
        
    auto tgtObj = objSystem->GetObject(tgt);
    auto tgtLoc = tgtObj->GetLocation();
    objHndl obstructor = objHndl::null;
    temple::GetRef<void(__cdecl)(objHndl, locXY, objHndl&)>(0x10058CA0)(handle, tgtLoc, obstructor);
    if (config.debugMessageEnable)
        logger->debug("TargetDistChecker: tgt2 is {}", obstructor);

    if (!obstructor
        || obstructor == tgt|| obstructor == handle)
    {
        return TRUE;
    }
        


    AnimPath animPath;
    animPath.flags = 0;
    animPath.pathLength = 0;
    animPath.fieldE4 = 0;
    animPath.fieldD4 = 0;
    animPath.fieldD8 = 0;
    animPath.fieldD0 = 0;
    //animPath.range = 200;

    auto animpathMaker = temple::GetRef<BOOL(__cdecl)(objHndl, locXY, AnimPath&, int)>(0x10017AD0);
    
    if (animpathMaker(handle, tgtLoc, animPath, 1)){
        if (config.debugMessageEnable)
            logger->debug("animpath successful");
        return TRUE;
    }
        
    if (config.debugMessageEnable)
        logger->debug("animpath failed!");
    return FALSE;
}

float AnimSystemHooks::HookedGetRunSpeed(objHndl handle, obj_f field){ // this is not a good fix since halflings are pre-assigned a faster move speed factor in the Protos

    auto obj = objSystem->GetObject(handle);
    // auto modelScale = obj->GetInt32(obj_f_model_scale) / 100.0f;
    auto val = obj->GetFloat(field);
    if (config.equalizeMoveSpeed && party.IsInParty(handle))
        return config.speedupFactor;
    return val;
}

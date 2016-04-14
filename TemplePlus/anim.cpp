#include "stdafx.h"
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

#pragma pack(push, 1)

// Has to be 0x10 in size
union AnimParam {
  objHndl obj;
  LocAndOffsets location;
  int number;
  int spellId;
};

struct AnimSlotGoalStackEntry {
  AnimGoalType goalType;
  int unk1;
  AnimParam self;
  AnimParam target;
  AnimParam block;
  AnimParam scratch;
  AnimParam parent;
  AnimParam targetTile;
  AnimParam range;
  AnimParam animId;
  AnimParam animIdPrevious;
  AnimParam animData;
  AnimParam spellData;
  AnimParam skillData;
  AnimParam flagsData;
  AnimParam scratchVal1;
  AnimParam scratchVal2;
  AnimParam scratchVal3;
  AnimParam scratchVal4;
  AnimParam scratchVal5;
  AnimParam scratchVal6;
  AnimParam soundHandle;
  uint32_t soundStreamId;
  uint32_t soundStreamId2; // Probably padding
  uint32_t padding[2];
  TimeEventObjInfo selfTracking;
  TimeEventObjInfo targetTracking;
  TimeEventObjInfo blockTracking;
  TimeEventObjInfo scratchTracking;
  TimeEventObjInfo parentTracking;
};

const auto TestSizeOfAnimSlotGoalStackEntry = sizeof(AnimSlotGoalStackEntry);

struct AnimSlot {
  AnimSlotId id;
  int flags; // See AnimSlotFlag
  int currentState;
  int field_14;
  GameTime nextTriggerTime;
  objHndl animObj;
  int currentGoal;
  int field_2C;
  AnimSlotGoalStackEntry goals[8];
  AnimSlotGoalStackEntry *pCurrentGoal;
  uint32_t unk1; // field_1134
  uint32_t unk2; // field_1138
  uint32_t unk3; // field_113C
  uint32_t padding[60];
  PathQueryResult path;
  AnimParam param1; // Used as parameters for goal states
  AnimParam param2; // Used as parameters for goal states
  uint32_t someGoalType;
  uint32_t unknown[5];
  uint64_t gametimeSth;
  uint32_t currentPing;    // I.e. used in
  uint32_t uniqueActionId; // ID assigned when triggered by a D20 action
};

struct AnimStateTransition {
  uint32_t newState;
  // Delay in milliseconds or one of the constants below
  int delay;

  // Delay is read from runSlot->animDelay
  static const uint32_t DelaySlot = -2;
  // Delay is read from 0x10307534 (written by some goal states)
  static const uint32_t DelayCustom = -3;
  // Specifies a random 0-300 ms delay
  static const uint32_t DelayRandom = -4;
};

struct AnimGoalState {
  BOOL(__cdecl *callback)(AnimSlot &slot);
  int argInfo1;
  int argInfo2;
  int refToOtherGoalType;
  AnimStateTransition afterFailure;
  AnimStateTransition afterSuccess;
};

struct AnimGoal {
  int statecount;
  AnimGoalPriority priority;
  int field_8;
  int field_C;
  int field_10;
  int relatedGoal1;
  int relatedGoal2;
  int relatedGoal3;
  AnimGoalState states[16];
  AnimGoalState state_special;
};
#pragma pack(pop)

const char *animGoalTypeNames[ag_count] = {
    "ag_animate",
    "ag_animate_loop",
    "ag_anim_idle",
    "ag_anim_fidget",
    "ag_move_to_tile",
    "ag_run_to_tile",
    "ag_attempt_move",
    "ag_move_to_pause",
    "ag_move_near_tile",
    "ag_move_near_obj",
    "ag_move_straight",
    "ag_attempt_move_straight",
    "ag_open_door",
    "ag_attempt_open_door",
    "ag_unlock_door",
    "ag_jump_window",
    "ag_pickup_item",
    "ag_attempt_pickup",
    "ag_pickpocket",
    "ag_attack",
    "ag_attempt_attack",
    "ag_talk",
    "ag_pick_weapon",
    "ag_chase",
    "ag_follow",
    "ag_follow_to_location",
    "ag_flee",
    "ag_throw_spell",
    "ag_attempt_spell",
    "ag_shoot_spell",
    "ag_hit_by_spell",
    "ag_hit_by_weapon",
	"ag_dodge", // This was missing from the name-table and was probably added for ToEE
    "ag_dying",
    "ag_destroy_obj",
    "ag_use_skill_on",
    "ag_attempt_use_skill_on",
    "ag_skill_conceal",
    "ag_projectile",
    "ag_throw_item",
    "ag_use_object",
    "ag_use_item_on_object",
    "ag_use_item_on_object_with_skill",
    "ag_use_item_on_tile",
    "ag_use_item_on_tile_with_skill",
    "ag_knockback",
    "ag_floating",
    "ag_close_door",
    "ag_attempt_close_door",
    "ag_animate_reverse",
    "ag_move_away_from_obj",
    "ag_rotate",
    "ag_unconceal",
    "ag_run_near_tile",
    "ag_run_near_obj",
    "ag_animate_stunned",
    "ag_animate_kneel_magic_hands",
    "ag_attempt_move_near",
    "ag_knock_down",
    "ag_anim_get_up",
    "ag_attempt_move_straight_knockback",
    "ag_wander",
    "ag_wander_seek_darkness",
    "ag_use_picklock_skill_on",
    "ag_please_move",
    "ag_attempt_spread_out",
    "ag_animate_door_open",
    "ag_animate_door_closed",
    "ag_pend_closing_door",
    "ag_throw_spell_friendly",
    "ag_attempt_spell_friendly",
    "ag_animate_loop_fire_dmg",
    "ag_attempt_move_straight_spell",
    "ag_move_near_obj_combat",
    "ag_attempt_move_near_combat",
    "ag_use_container",
    "ag_throw_spell_w_cast_anim",
    "ag_attempt_spell_w_cast_anim",
    "ag_throw_spell_w_cast_anim_2ndary",
    "ag_back_off_from",
    "ag_attempt_use_pickpocket_skill_on",
    "ag_use_disable_device_skill_on_data"
};

ostream &operator<<(ostream &str, const AnimSlotId &id) {
  str << id.ToString();
  return str;
}

std::string GetAnimGoalTypeName(AnimGoalType type) {
  auto i = (size_t)type;
  if (type < ag_count) {
    return animGoalTypeNames[type];
  }
  return fmt::format("Unknown Goal Type [{}]", i);
}

ostream &operator<<(ostream &str, AnimGoalType type) {
  size_t i = (size_t)type;
  if (type < ag_count) {
    str << animGoalTypeNames[type];
  } else {
    str << fmt::format("Unknown Goal Type [{}]", i);
  }
  return str;
}

enum AnimGoalDataItem {
  AGDATA_SELF_OBJ = 0,     // Type: 1 (Object)
  AGDATA_TARGET_OBJ,       // Type: 1
  AGDATA_BLOCK_OBJ,        // Type: 1
  AGDATA_SCRATCH_OBJ,      // Type: 1
  AGDATA_PARENT_OBJ,       // Type: 1
  AGDATA_TARGET_TILE,      // Type: 2 (Location)
  AGDATA_RANGE_DATA,       // Type: 2
  AGDATA_ANIM_ID,          // Type: 0 (just a 32-bit number it seems)
  AGDATA_ANIM_ID_PREVIOUS, // Type: 0
  AGDATA_ANIM_DATA,        // Type: 0
  AGDATA_SPELL_DATA,       // Type: 0
  AGDATA_SKILL_DATA,       // Type: 0
  AGDATA_FLAGS_DATA,       // Type: 0
  AGDATA_SCRATCH_VAL1,     // Type: 0
  AGDATA_SCRATCH_VAL2,     // Type: 0
  AGDATA_SCRATCH_VAL3,     // Type: 0
  AGDATA_SCRATCH_VAL4,     // Type: 0
  AGDATA_SCRATCH_VAL5,     // Type: 0
  AGDATA_SCRATCH_VAL6,     // Type: 0
  AGDATA_SOUND_HANDLE      // Type: 0
};

const char *AnimGoalDataNames[] = {
    "AGDATA_SELF_OBJ",     "AGDATA_TARGET_OBJ",   "AGDATA_BLOCK_OBJ",
    "AGDATA_SCRATCH_OBJ",  "AGDATA_PARENT_OBJ",   "AGDATA_TARGET_TILE",
    "AGDATA_RANGE_DATA",   "AGDATA_ANIM_ID",      "AGDATA_ANIM_ID_PREV",
    "AGDATA_ANIM_DATA",    "AGDATA_SPELL_DATA",   "AGDATA_SKILL_DATA",
    "AGDATA_FLAGS_DATA",   "AGDATA_SCRATCH_VAL1", "AGDATA_SCRATCH_VAL2",
    "AGDATA_SCRATCH_VAL3", "AGDATA_SCRATCH_VAL4", "AGDATA_SCRATCH_VAL5",
    "AGDATA_SCRATCH_VAL6", "AGDATA_SOUND_HANDLE"};

enum AnimSlotFlag {
  ASF_ACTIVE = 1,
  ASF_UNK2 = 2, // Used in context with "killing the animation slot"
  ASF_UNK3 = 4, // Seen in goalstatefunc_82, goalstatefunc_83, set with 0x8 in
                // goalstatefunc_42
  ASF_UNK4 = 8, // Seen in goalstatefunc_82, goalstatefunc_83, set with 0x8 in
                // goalstatefunc_42
  ASF_UNK5 = 0x10, // Seen in goalstatefunc_84_animated_forever, set in
                   // goalstatefunc_87
  ASF_UNK7 =
      0x20, // Seen as 0x30 is cleared in goalstatefunc_7 and goalstatefunc_8
  ASF_RUNNING = 0x40,
  ASF_SPEED_RECALC = 0x80,
  ASF_UNK8 = 0x400,   // Seen in goal_calc_path_to_loc, goalstatefunc_13_rotate,
                      // set in goalstatefunc_18
  ASF_UNK10 = 0x800,  // Seen in goalstatefunc_37
  ASF_UNK9 = 0x4000,  // Seen in goalstatefunc_19
  ASF_UNK11 = 0x8000, // Test in goalstatefunc_43
  ASF_UNK1 = 0x10000,
  ASF_UNK6 =
      0x20000, // Probably sound related (seen in anim_goal_free_sound_id)
  ASF_UNK12 = 0x40000 // set goalstatefunc_48, checked in goalstatefunc_50
};

static void assertStructSizes() {
  static_assert(temple::validate_size<ObjectId, 0x18>::value,
                "Goal Stack entry has incorrect size.");
  static_assert(temple::validate_size<AnimParam, 0x10>::value,
                "Anim Param union has the wrong size.");
  static_assert(temple::validate_size<AnimSlotGoalStackEntry, 0x220>::value,
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

  /*
  Interrupts animations in the given animation slot. Exact behaviour is not
  known yet.
  */
  void(__cdecl *Interrupt)(const AnimSlotId &id, AnimGoalPriority priority);

  int (*anim_first_run_idx_for_obj)(objHndl obj);
  BOOL (*anim_run_id_for_obj)(objHndl obj, AnimSlotId *slotIdOut);

  AnimSlotId tmpId;

  AnimAddressTable() {
    rebase(GetAnimName, 0x102629D0);
    rebase(nextUniqueId, 0x11E61520);
    rebase(slotsInUse, 0x10AA4BBC);
    rebase(customDelayInMs, 0x10307534);

    rebase(PushGoalDying, 0x100157B0);
    rebase(InterruptAllForTbCombat, 0x1000C950);
    rebase(Interrupt, 0x10056090);

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
  }

} addresses;

AnimationGoals animationGoals;

bool AnimationGoals::PushRotate(objHndl obj, float rotation) {
  return addresses.PushRotate(obj, rotation);
}

bool AnimationGoals::PushUseSkillOn(objHndl actor, objHndl target,
                                    SkillEnum skill, objHndl scratchObj,
                                    int goalFlags) {
  return addresses.PushUseSkillOn(actor, target, scratchObj, skill, goalFlags);
}

bool AnimationGoals::PushRunNearTile(objHndl actor, LocAndOffsets target,
                                     int radiusFeet) {
  return addresses.PushRunNearTile(actor, target, radiusFeet);
}

bool AnimationGoals::PushUnconceal(objHndl actor) {
  return addresses.PushUnconceal(actor);
}

bool AnimationGoals::Interrupt(objHndl actor, AnimGoalPriority priority,
                               bool all) {
  return addresses.Interrupt(actor, priority, all);
}

void AnimationGoals::PushFallDown(objHndl actor, int unk) {
  addresses.PushFallDown(actor, unk);
}

int AnimationGoals::PushAttackAnim(objHndl actor, objHndl target, int unk1,
                                   int hitAnimIdx, int playCrit,
                                   int useSecondaryAnim) {
  return addresses.PushAttackAnim(actor, target, unk1, hitAnimIdx, playCrit,
                                  useSecondaryAnim);
}

int AnimationGoals::GetAnimIdSthgSub_1001ABB0(objHndl objHndl) {
  return addresses.GetAnimIdSthgSub_1001ABB0(objHndl);
}

int AnimationGoals::PushAttemptAttack(objHndl attacker, objHndl defender) {
  return addresses.PushAttemptAttack(attacker, defender);
}

int AnimationGoals::PushAnimate(objHndl obj, int anim) {
  return addresses.PushAnimate(obj, anim);
}

//*****************************************************************************
//* Anim
//*****************************************************************************

AnimSystem::AnimSystem(const GameSystemConf &config) {
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

void AnimSystem::ProcessAnimEvent(const TimeEvent *evt) {

  if (mAllSlotsUsed) {
    static auto anim_goal_interrupt_all_goals_of_priority =
        temple::GetPointer<signed int(signed int priority)>(0x1000c8d0);
    anim_goal_interrupt_all_goals_of_priority(AGP_3);
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
    return;
  }

  // Slot did belong to "us", but it was deactivated earlier
  if (!(slot.flags & ASF_ACTIVE)) {
    ProcessActionCallbacks();
    return;
  }

  // Interesting how this reschedules in at least 100ms which seems steep for
  // animation processing
  // Have to check where and why this is set
  if (slot.flags & ASF_UNK1) {
    ProcessActionCallbacks();

    auto delay = std::max(slot.path.someDelay, 100);
    RescheduleEvent(delay, slot, evt);
    return;
  }

  if (slot.currentGoal < 0) {
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

  // And another safeguard
  if (currentGoal->goalType < 0 || currentGoal->goalType >= ag_count) {
    slot.flags |= ASF_UNK2;
    stopProcessing = true;
  } else {
    goal = mGoals[currentGoal->goalType];
    if (!goal) {
      logger->error("Animation slot {} references null goal {}.", slot.id,
                    currentGoal->goalType);
    }
  }

  // This validates object references found in the animation slot
  if (!PrepareSlotForGoalState(slot, nullptr)) {
    ProcessActionCallbacks();
    return;
  }

  // Validates that the object the animation runs for is not destroyed
  if (slot.animObj) {
    if (objects.GetFlags(slot.animObj) & OF_DESTROYED) {
      logger->warn("Processing animation slot {} for destroyed object.",
                   slot.id);
    }
  } else {
    // Animation is no longer associated with an object after validation
    slot.flags |= ASF_UNK2;
    stopProcessing = true;
  }

  int delay = 0;

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
        templeFuncs.TurnProcessing(slot.animObj);
        mCurrentlyProcessingSlotIdx = -1;
        animAddresses.Interrupt(slot.id, AGP_HIGHEST);
        ProcessActionCallbacks();
        return;
      }

      auto &currentState = goal->states[slot.currentState];

      // Prepare for the current state
      if (!PrepareSlotForGoalState(slot, &currentState)) {
        ProcessActionCallbacks();
        return;
      }

      auto stateResult = currentState.callback(slot);

      // Check flags on the slot that may have been set by the callbacks.
      if (slot.flags & ASF_UNK1) {
        stopProcessing = true;
      }

      if (!(slot.flags & ASF_ACTIVE)) {
        mCurrentlyProcessingSlotIdx = -1;
        ProcessActionCallbacks();
        return;
      }

      if (slot.flags & ASF_UNK2) {
        break;
      }

      auto &transition =
          stateResult ? currentState.afterSuccess : currentState.afterFailure;
      auto nextState = transition.newState;
      delay = transition.delay;

      // Special transitions
      if (nextState & 0xFF000000) {
        if (nextState & 0x10000000) {
          slot.currentState = 0;
          stopProcessing = true;
        }
        if ((nextState & 0x30000000) == 0x30000000) {
          auto newState = currentGoal;
          auto newGoal = &goal;
          auto popFlags = nextState;
          PopGoal(slot, popFlags, newGoal, &currentGoal, &stopProcessing);
          if (nextState & 0x08000000) {
            PopGoal(slot, popFlags, newGoal, &currentGoal, &stopProcessing);
          }
        }
        if (nextState & 0x40000000) {
			if (slot.currentGoal >= 7) {
				logger->error("Unable to push goal, because anim slot %s has overrun!", slot.id.ToString());
				logger->error("Current sub goal stack is:");
				
				for (auto i = 0; i < slot.currentGoal; i++) {
					logger->info("\t[{}]: Goal {}", i, animGoalTypeNames[slot.goals[i].goalType]);
				}

				slot.flags |= ASF_UNK2;
				slot.currentState = 0;
				stopProcessing = true;
			} else {
				slot.currentState = 0;
				slot.currentGoal++;

				currentGoal = &slot.goals[slot.currentGoal];
				slot.pCurrentGoal = currentGoal;

				// Apparently if 0x30 00 00 00 is also set, it copies the previous goal????
				if (slot.currentGoal > 0 && (nextState & 0x30000000) != 0x30000000) {
					slot.goals[slot.currentGoal] = slot.goals[slot.currentGoal - 1];
				}

				auto newGoalType = (AnimGoalType)(nextState & 0xFFF);
				goal = mGoals[newGoalType];
				slot.goals[slot.currentGoal].goalType = newGoalType;

				static auto animNumActiveGoals_inc = temple::GetPointer<void(AnimSlot &slot, const AnimGoal *pGoalNode)>(0x10055bf0);
				animNumActiveGoals_inc(slot, goal);
			}
        }
        if ((nextState & 0x90000000) == 0x90000000) {
          auto prio = mGoals[slot.goals[slot.currentGoal].goalType]->priority;
          if (prio < AnimGoalPriority::AGP_MAX) {
            slot.flags |= 2u;
            for (auto i = 1; i < slot.currentGoal; i++) {
              auto goal = mGoals[slot.goals[i].goalType];

              if (goal->state_special.callback) {
                if (PrepareSlotForGoalState(slot, &goal->state_special)) {
                  goal->state_special.callback(slot);
                }
              }
            }

            auto v33 = mGoals[slot.goals[0].goalType];
            if (v33->state_special.callback) {
              if (PrepareSlotForGoalState(slot, &v33->state_special))
                v33->state_special.callback(slot);
            }
            slot.unk2 |= 1u;
            slot.currentState = 0;
            slot.path.flags &= 0xFFFFFFFE;
            GoalDestinationsRemove(slot.path.mover);
            slot.field_14 = -1;
            stopProcessing = true;
          } else {
            currentGoal = &slot.goals[slot.currentGoal];
            goal = mGoals[currentGoal->goalType];
            while (goal->priority < AnimGoalPriority::AGP_MAX) {
              PopGoal(slot, 0x30000000, &goal, &currentGoal, &stopProcessing);
              currentGoal = &slot.goals[slot.currentGoal];
              goal = mGoals[currentGoal->goalType];
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

  // Does Flag 2 mean "COMPLETED" ?
  if (!(slot.flags & ASF_UNK2)) {
    if (slot.flags & ASF_ACTIVE) {
      // This actually seems to be the "GOOD" case
      RescheduleEvent(delay, slot, evt);
    }
    ProcessActionCallbacks();
    return;
  }

  if (slot.animObj) {
    // Interrupt everything for the slot
    animAddresses.Interrupt(slot.id, AGP_HIGHEST);

	// The slot may actually be deallocated by interrupt above
    if (slot.animObj && objects.IsCritter(slot.animObj)) {
      PushActionCallback(slot);
    }
  }

  ProcessActionCallbacks();
}

void AnimSystem::ProcessActionCallbacks() {
  for (auto &callback : mActionCallbacks) {
    actSeqSys.PerformOnAnimComplete(callback.obj, callback.uniqueId);
  }

  mActionCallbacks.clear();
}

void AnimSystem::PushActionCallback(AnimSlot &slot) {

  if (slot.uniqueActionId == 0) {
    return;
  }

  mActionCallbacks.push_back({slot.animObj, slot.uniqueActionId});
}

void AnimSystem::PopGoal(AnimSlot &slot, uint32_t popFlags,
                         const AnimGoal **newGoal,
                         AnimSlotGoalStackEntry **newCurrentGoal,
                         bool *stopProcessing) {
  if (!slot.currentGoal && !(popFlags & 0x40000000)) {
    slot.flags |= AnimSlotFlag::ASF_UNK2;
  }

  if ((*newGoal)->state_special.callback) {
    if (!(popFlags & 0x70000000) || !(popFlags & 0x4000000)) {
      if (PrepareSlotForGoalState(slot, &(*newGoal)->state_special)) {
        (*newGoal)->state_special.callback(slot);
      }
    }
  }

  if (!(popFlags & 0x1000000)) {
    slot.flags &= ~0x83C;
    slot.padding[54] = 0; // slot->anim_path.maxPathLength = 0;
  }

  if (popFlags & 0x2000000) {
    objHndl mover = slot.path.mover;
    slot.unk2 = 1; // slot->anim_path.field_0 = 1;
    slot.path.flags = PF_NONE;
    GoalDestinationsRemove(mover);
  }

  static auto animNumActiveGoals_dec =
      temple::GetRef<void(__cdecl)(AnimSlot &, const AnimGoal *)>(0x10055CA0);
  animNumActiveGoals_dec(slot, *newGoal);
  slot.currentGoal--;
  slot.currentState = 0;
  if (slot.currentGoal < 0) {
    if (!(popFlags & 0x40000000)) {
      slot.flags |= AnimSlotFlag::ASF_UNK2;
    }
  } else {
    auto prevGoal = slot.pCurrentGoal;
    slot.pCurrentGoal = *newCurrentGoal = &slot.goals[slot.currentGoal];
    *newGoal = mGoals[(*newCurrentGoal)->goalType];
    *stopProcessing = false;
    if (prevGoal->goalType == ag_anim_fidget) {
      // FIX: prevents ag_anim_fidget from queueing an AnimComplete call (which
      // creates the phantom animId = 0 bullshit)
    } else if ((*newCurrentGoal)->goalType == ag_anim_idle &&
               !(popFlags & 0x40000000)) {
      PushActionCallback(slot);
    }
  }
}

/*
When an event should be re-executed at a later time, but unmodified, this
method is used. It also checks whether animations should "catch up" (by skipping
frames essentially), or whether they should be run at whatever speed was
intended,
but visibly slowing down.
*/
void AnimSystem::RescheduleEvent(int delayMs, AnimSlot &slot,
                                 const TimeEvent *oldEvt) {
  TimeEvent evt;
  evt.system = TimeEventType::Anim;
  evt.params[0].int32 = slot.id.slotIndex;
  evt.params[1].int32 = slot.id.uniqueId;
  evt.params[2].int32 =
      1111; // Some way to identify these rescheduled events???

  if (config.animCatchup) {
    gameSystems->GetTimeEvent().ScheduleAbsolute(evt, oldEvt->time, delayMs,
                                                 &slot.nextTriggerTime);
  } else {
    gameSystems->GetTimeEvent().Schedule(evt, delayMs, &slot.nextTriggerTime);
  }
}

void AnimSystem::GoalDestinationsRemove(objHndl obj) {
  static auto goal_destinations_remove =
      temple::GetPointer<void(objHndl)>(0x100bac80);
  goal_destinations_remove(obj);
}

bool AnimSystem::InterruptGoals(AnimSlot &slot, AnimGoalPriority priority) {

  if (!slot.flags & ASF_ACTIVE) {
    return false;
  }

  if (!gameSystems->IsResetting() && slot.currentGoal != -1) {
    auto &stackTop = slot.goals[slot.currentGoal];
    auto goal = mGoals[stackTop.goalType];
    assert(goal);

    if (priority < AnimGoalPriority::AGP_HIGHEST && goal->field_8) {
      return true;
    }
    if (priority == AnimGoalPriority::AGP_5 && !goal->field_8) {
      return false;
    }

    if (goal->priority == AnimGoalPriority::AGP_3) {
      if (priority < AnimGoalPriority::AGP_3) {
        return false;
      }
    } else if (goal->priority == AnimGoalPriority::AGP_2) {
      if (priority < 2) {
        return false;
      }
    } else if (goal->priority >= priority) {
      if (goal->priority != AnimGoalPriority::AGP_MAX) {
        return false;
      }
      slot.flags &= ASF_UNK5;
    }
  }

  auto goalType = mGoals[slot.goals[0].goalType];
  if (goalType->priority >= AnimGoalPriority::AGP_MAX &&
      priority < AnimGoalPriority::AGP_MAX) {
    auto pNewStackTopOut = &slot.goals[slot.currentGoal];
    for (goalType = mGoals[pNewStackTopOut->goalType];
         goalType->priority < AnimGoalPriority::AGP_MAX;
         goalType = mGoals[pNewStackTopOut->goalType]) {
      bool stopProcessing = false;
      PopGoal(slot, 0x30000000, &goalType, &pNewStackTopOut, &stopProcessing);
      pNewStackTopOut = &slot.goals[slot.currentGoal];
    }
    return true;
  }

  slot.flags |= ASF_UNK2;

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
      auto goal = mGoals[slot.goals[i].goalType];
      if (!gameSystems->IsResetting()) {
        if (goal->state_special.callback) {
          if (!PrepareSlotForGoalState(slot, &goal->state_special)) {
            goal->state_special.callback(slot);
          }
        }
      }
    }
  }

  static auto deallocateSlot = temple::GetPointer<BOOL(AnimSlot &)>(0x10055d30);
  deallocateSlot(slot);

  return false;
}

bool AnimSystem::PrepareSlotForGoalState(AnimSlot &slot,
                                         const AnimGoalState *state) {
  static auto anim_prepare_run_for_goalstate =
      temple::GetPointer<int(AnimSlot * runSlot, const AnimGoalState *state)>(
          0x10055700);
  return anim_prepare_run_for_goalstate(&slot, state) == TRUE;
}

std::string AnimSlotId::ToString() const {
  return format("[{}:{}r{}]", slotIndex, uniqueId, field_8);
}

static BOOL goalstatefunc_106(AnimSlot &slot) {
  auto obj = slot.param1.obj;
  assert(slot.param1.obj);

  auto aasHandle = objects.GetAnimHandle(obj);
  assert(aasHandle);

  if (objects.getInt32(obj, obj_f_spell_flags) & 0x10000) {
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
        temple::GetPointer<BOOL(objHndl obj, AnimSlot & runSlot,
                                uint32_t animHandle, uint32_t * eventOut)>(
            0x10016530);
    uint32_t eventOut = 0;
    anim_frame_advance_maybe(obj, slot, aasHandle->GetHandle(), &eventOut);

    // This is the ACTION trigger
    if (eventOut & 1) {
      slot.flags |= 4u;
      return TRUE;
    }

	// If the animation is a looping animation, it does NOT have a
	// defined end, so we just trigger the end here anyway otherwise
	// this'll loop endlessly
	bool looping = false;
	if (animId.IsWeaponAnim() && animId.GetWeaponAnim() == gfx::WeaponAnim::Idle) {
		// We will continue anyway down below, because the character is idling, so log a message
		if (!(eventOut & 2)) {
			logger->info("Ending wait for animation action/end in goal {}, because the idle animation would never end.",
				animGoalTypeNames[slot.pCurrentGoal->goalType]);
		}
		looping = true;
	}

    // This is the END trigger
    if (!looping & !(eventOut & 2))
      return TRUE;

    // Clears WaypointDelay flag
    auto gameObj = objSystem->GetObject(obj);
    if (objects.IsNPC(obj)) {
      auto flags = gameObj->GetInt64(obj_f_npc_ai_flags64);
      gameObj->SetInt64(obj_f_npc_ai_flags64, flags & 0xFFFFFFFFFFFFFFFDui64);
    }

    // Clear 0x10 slot flag
    slot.flags &= 0xFFFFFFEF;
  }

  return FALSE;
}

static BOOL goalstatefunc_83(AnimSlot &slot) {
  auto flags = slot.flags;
  if (flags & 4 && !(flags & 8)) {
    slot.flags = flags | 8;
    return TRUE;
  } else {
    return FALSE;
  }
}

static BOOL goalstatefunc_45(AnimSlot &slot) {
  auto obj = slot.param1.obj;
  assert(obj);
  auto aasHandle = objects.GetAnimHandle(obj);
  assert(aasHandle);

  auto animId = aasHandle->GetAnimId();
  return animId.IsConjuireAnimation() ? TRUE : FALSE;
}

static BOOL goalstatefunc_41(AnimSlot &slot) {
  auto spell = spellsCastRegistry.get(slot.param1.number);
  return FALSE;
}

static BOOL goalstatefunc_42(AnimSlot &slot) {
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
  const char *name() override { return "Anim Fix"; }

  static void Dump();

  void apply() override {

    /*replaceFunction<int(const TimeEvent *)>(
        0x1001B830, [](const TimeEvent *evt) -> int {
          gameSystems->GetAnim().ProcessAnimEvent(evt);
          return TRUE;
        });*/

	// AnimSlotInterruptGoals
	replaceFunction<BOOL(AnimSlotId &, AnimGoalPriority)>(0x10056090, [](AnimSlotId &animId, AnimGoalPriority priority) {
		auto &slot = gameSystems->GetAnim().mSlots[animId.slotIndex];
		gameSystems->GetAnim().InterruptGoals(slot, priority);
		return TRUE;
	});

    // anim_pop_goal
    replaceFunction<void(AnimSlot &, const uint32_t *, const AnimGoal **,
                         AnimSlotGoalStackEntry **, BOOL *)>(
        0x10016FC0,
        [](AnimSlot &slot, const uint32_t *popFlags, const AnimGoal **newGoal,
           AnimSlotGoalStackEntry **newCurrentGoal, BOOL *stopProcessing) {
          bool stopProcessingBool = *stopProcessing == TRUE;
          gameSystems->GetAnim().PopGoal(slot, *popFlags, newGoal,
                                         newCurrentGoal, &stopProcessingBool);
          *stopProcessing = stopProcessingBool ? TRUE : FALSE;
        });

    // goalstatefunc_106
    replaceFunction<BOOL(AnimSlot &)>(0x100185e0, goalstatefunc_106);
    // goalstatefunc_83_checks_flag4_set_flag8
    replaceFunction<BOOL(AnimSlot &)>(0x10012c80, goalstatefunc_83);
    // goalstatefunc_45
    replaceFunction<BOOL(AnimSlot &)>(0x10010520, goalstatefunc_45);
    // goalstatefunc_41
    replaceFunction<BOOL(AnimSlot &)>(0x10010290, goalstatefunc_41);
    // goalstatefunc_42
    replaceFunction<BOOL(AnimSlot &)>(0x100102c0, goalstatefunc_42);

    // Register a debug function for dumping the anims
    RegisterDebugFunction("dump_anim_goals", Dump);
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
                            delay, animGoalTypeNames[newGoal]);
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
      {"refToOtherGoalType", state.refToOtherGoalType}};

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

void AnimSystemHooks::Dump() {

  map<uint32_t, string> goalFuncNames;
  map<uint32_t, string> goalFuncDescs;
#define MakeName(a, b) goalFuncNames[a] = b
#define MakeDesc(a, b) goalFuncDescs[a] = b
#include "goalfuncnames.h"

  auto outputFolder = GetUserDataFolder() + L"animationGoals";
  CreateDirectory(outputFolder.c_str(), NULL);

  auto animSys = gameSystems->GetAnim();

  using namespace json11;
  Json::array goalsArray;

  for (int i = 0; i < ag_count; ++i) {
    auto goal = animSys.mGoals[i];
    if (!goal)
      continue;
    auto goalName = animGoalTypeNames[i];

    Json::object goalObj{{"id", i},
                         {"name", goalName},
                         {"priority", (int)goal->priority},
                         {"field8", goal->field_8},
                         {"fieldc", goal->field_C},
                         {"field10", goal->field_10},
                         {"relatedGoal1", goal->relatedGoal1},
                         {"relatedGoal2", goal->relatedGoal2},
                         {"relatedGoal3", goal->relatedGoal3}};
    if (goal->state_special.callback) {
      auto specialState =
          StateToJson(goal->state_special, goalFuncNames, goalFuncDescs);
      // Those are not used for the cleanup
      specialState.erase("transition");
      specialState.erase("trueTransition");
      specialState.erase("falseTransition");
      specialState.erase("refToOtherGoalType");
      goalObj["specialState"] = specialState;
    }

    std::vector<Json::object> states;

    string diagramText = "@startuml\n";

    diagramText += "[*] --> state0\n";

    for (int j = 0; j < goal->statecount; ++j) {
      auto &state = goal->states[j];

      states.push_back(StateToJson(state, goalFuncNames, goalFuncDescs));

      auto stateText = "**" + goalFuncNames[(uint32_t)state.callback] + "**";
      auto stateDesc = goalFuncDescs[(uint32_t)state.callback];

      if (state.callback && stateText == "****") {
        logger->warn("No name for goal func {}", (void *)state.callback);
      }

      auto param1 = state.argInfo1;
      if (param1 != -1) {
        stateText += "\\nParam 1: " + GetAnimParamName(param1);
      }

      auto param2 = state.argInfo2;
      if (param2 != -1) {
        stateText += "\\nParam 2:" + GetAnimParamName(param2);
      }

      if (state.afterSuccess.newState == state.afterFailure.newState &&
          state.afterSuccess.delay == state.afterFailure.delay) {
        getTransitionText(diagramText, j, state.afterSuccess, "always");
      } else {
        getTransitionText(diagramText, j, state.afterSuccess, "true");
        getTransitionText(diagramText, j, state.afterFailure, "false");
      }

      diagramText += format("state \"{}\" as state{}\n", stateText, j);
      if (!stateDesc.empty()) {
        diagramText += format("state{} : {}\n", j, stateDesc);
      }
    }

    goalObj["states"] = states;
    goalsArray.push_back(goalObj);

    diagramText += "\n@enduml\n";

    ofstream o(format(L"{}/{:02d}_{}.txt", outputFolder, i, goalName));
    o << diagramText;
  }

  ofstream jsonO(format(L"{}/goals.json", outputFolder));
  jsonO << Json(goalsArray).dump();

  logger->info("DONE");
}

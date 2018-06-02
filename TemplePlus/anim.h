
#pragma once
#include "obj.h"
#include "skill.h"
#include "gamesystems/gamesystem.h"
#include <fmt/format.h>

struct AnimPath;
struct AnimSlot;
struct AnimGoal;
struct AnimSlotGoalStackEntry;
struct AnimGoalState;

#pragma pack(push, 1)
struct AnimSlotId {
	int slotIndex;
	int uniqueId;
	int field_8;

	std::string ToString() const;
};
#pragma pack(pop)

// Allows for direct use of AnimSlotId in format() strings
inline void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const AnimSlotId &id) {
	f.writer().write("{}", id.ToString());
}

enum AnimGoalType : uint32_t {
	ag_animate = 0x0,
	ag_animate_loop = 0x1,
	ag_anim_idle = 0x2,
	ag_anim_fidget = 0x3,
	ag_move_to_tile = 0x4,
	ag_run_to_tile = 0x5,
	ag_attempt_move = 0x6,
	ag_move_to_pause = 0x7,
	ag_move_near_tile = 0x8,
	ag_move_near_obj = 0x9,
	ag_move_straight = 0xA,
	ag_attempt_move_straight = 0xB,
	ag_open_door = 0xC,
	ag_attempt_open_door = 0xD,
	ag_unlock_door = 0xE,
	ag_jump_window = 0xF,
	ag_pickup_item = 0x10,
	ag_attempt_pickup = 0x11,
	ag_pickpocket = 0x12,
	ag_attack = 0x13,
	ag_attempt_attack = 0x14,
	ag_talk = 0x15,
	ag_pick_weapon = 0x16,
	ag_chase = 0x17,
	ag_follow = 0x18,
	ag_follow_to_location = 0x19,
	ag_flee = 0x1A,
	ag_throw_spell = 0x1B,
	ag_attempt_spell = 0x1C,
	ag_shoot_spell = 0x1D,
	ag_hit_by_spell = 0x1E,
	ag_hit_by_weapon = 0x1F,
	ag_dodge = 0x20, // this is where the enums start to be off compared to the debug string in the dll (they added ag_dodge and didn't update the list)
	ag_dying,
	ag_destroy_obj,
	ag_use_skill_on,
	ag_attempt_use_skill_on,
	ag_skill_conceal,
	ag_projectile,
	ag_throw_item,
	ag_use_object,
	ag_use_item_on_object,
	ag_use_item_on_object_with_skill,
	ag_use_item_on_tile,
	ag_use_item_on_tile_with_skill,
	ag_knockback,
	ag_floating,
	ag_close_door,
	ag_attempt_close_door,
	ag_animate_reverse,
	ag_move_away_from_obj,
	ag_rotate,
	ag_unconceal,
	ag_run_near_tile,
	ag_run_near_obj,
	ag_animate_stunned,
	ag_animate_kneel_magic_hands,
	ag_attempt_move_near,
	ag_knock_down,
	ag_anim_get_up,
	ag_attempt_move_straight_knockback,
	ag_wander,
	ag_wander_seek_darkness,
	ag_use_picklock_skill_on,
	ag_please_move,
	ag_attempt_spread_out,
	ag_animate_door_open,
	ag_animate_door_closed,
	ag_pend_closing_door,
	ag_throw_spell_friendly,
	ag_attempt_spell_friendly,
	ag_animate_loop_fire_dmg,
	ag_attempt_move_straight_spell,
	ag_move_near_obj_combat,
	ag_attempt_move_near_combat,
	ag_use_container,
	ag_throw_spell_w_cast_anim,
	ag_attempt_spell_w_cast_anim,
	ag_throw_spell_w_cast_anim_2ndary,
	ag_back_off_from,
	ag_attempt_use_pickpocket_skill_on,
	ag_use_disable_device_skill_on_data,
	ag_count = 82
};

std::string GetAnimGoalTypeName(AnimGoalType type);

ostream &operator<<(ostream &str, AnimGoalType type);

struct PathQueryResult;

enum AnimGoalPriority {
	AGP_NONE = 0,
	AGP_1 = 1,
	AGP_2 = 2,
	AGP_3 = 3,
	AGP_4 = 4,
	AGP_5 = 5,
	AGP_HIGHEST = 6,
	AGP_7 = 7,	
	AGP_MAX = 8
};

class AnimationGoals {
friend class AnimSystemHooks;
friend class AnimSystem;
public:

	/*
	   Pushes fidget anim goal
	 */
	BOOL PushFidget(objHndl handle);

	/*
		Pushes a goal for the obj that will rotate it to the given rotation using its rotation speed.
		Please note that the target rotation is absolute, not relative.
	*/
	bool PushRotate(objHndl obj, float rotation);

	/*
		Pushes a goal for the actor to use a certain skill on the given target.
	*/
	bool PushUseSkillOn(objHndl actor, objHndl target, SkillEnum skill, objHndl scratchObj = objHndl::null, int goalFlags = 0);

	/*
		Pushes a goal for the actor to run near a certain tile.
	*/
	bool PushRunNearTile(objHndl actor, LocAndOffsets target, int radiusFeet);

	bool PushRunToTile(objHndl handle, LocAndOffsets loc, PathQueryResult *pqr = nullptr);

	bool ShouldRun(objHndl handle); // should critter use ag_run_to_tile?
	bool PushMoveToTile(objHndl handle, LocAndOffsets loc);
	bool PushWalkToTile(objHndl handle, LocAndOffsets loc);
	void SetRunningState(bool state); // sets ASF_RUNNING on/off on the last anim slot (i.e. should characters use running animation, as opposed to walking)
	void TurnOnRunning(); // same as SetRunningState, except for the global anim

	/*
		Pushes a goal to play the unconceal animation and unconceal the critter.
	*/
	bool PushUnconceal(objHndl actor);

	/*
		Interrupts currently running animation goals up to the given priority.
	*/
	bool Interrupt(objHndl actor, AnimGoalPriority priority, bool all = false);

	void PushFallDown(objHndl actor, int unk);

	int PushAttackAnim(objHndl actor, objHndl target, int unk1, int hitAnimIdx, int playCrit, int useSecondaryAnim);
	int GetActionAnimId(objHndl objHndl);
	int PushAttemptAttack(objHndl attacker, objHndl defender);
	int PushDodge(objHndl attacker, objHndl dodger);
	int PushAnimate(objHndl obj, int anim);

	/*
	pushes spell animation, including wand animation if relevant
	*/
	BOOL PushSpellCast(SpellPacketBody &spellPkt, objHndl item);

	BOOL PushSpellInterrupt(const objHndl& caster, objHndl item,  AnimGoalType animGoalType, int spellSchool);

	/*
		used by the general out-of-combat mouse LMB click handler
	*/
	void PushForMouseTarget(objHndl handle, AnimGoalType type, objHndl tgt, locXY loc, objHndl scratchObj, int someFlag);

	BOOL IsObjDoingRelatedGoal(objHndl handle, AnimGoalType type, AnimSlotId& slotId);

	void Debug();
	const AnimGoal* GetGoal(AnimGoalType goalType);
	
	void GoalDestinationRemove(objHndl);
	void GoalDestinationAdd(objHndl handle, LocAndOffsets loc);
	void SetRuninfoDeallocCallback(void(__cdecl* cb)());
	bool InterruptAllForTbCombat();
private:
	BOOL GetSlot(AnimSlotId* runId, AnimSlot **runSlotOut);
};

extern AnimationGoals animationGoals;

struct AnimActionCallback {
	objHndl obj;
	uint32_t uniqueId;
};

struct AnimPath
{
	int flags;
	int8_t deltas[200]; // xy delta pairs describing deltas for drawing a line in screenspace
	int range;
	int fieldD0;
	int fieldD4;
	int fieldD8;
	int fieldDC;
	int pathLength;
	int fieldE4;
	locXY objLoc;
	locXY tgtLoc;
};

const int testSizeofAnimPath = sizeof(AnimPath); //should be 248 (0xF8)

struct AnimPathSpec
{
	objHndl handle;
	locXY srcLoc;
	locXY destLoc;
	int size;
	int8_t * deltas;
	int flags;
	int distTiles;

	BOOL PathSearch();
};

using AnimSlotArray = AnimSlot[512];
using AnimGoalArray = const AnimGoal*[82];

struct TimeEvent;
class AnimSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
friend class AnimSystemHooks;
friend class AnimationGoals;
friend struct AnimSlotGoalStackEntry;
public:
	static constexpr auto Name = "Anim";
	AnimSystem(const GameSystemConf &config);
	~AnimSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	void ClearGoalDestinations();
	void InterruptAll();

	BOOL ProcessAnimEvent(const TimeEvent *evt);

	void PushDisableFidget();
	void PopDisableFidget();

	void StartFidgetTimer();
	int GetRunSlotIdForObj(objHndl handle);
	AnimSlot & GetRunSlot(int slotId);
	BOOL GetRunSlotId(objHndl handle, AnimSlotId *);

private:
	/*
		Set to true when ToEE cannot allocate an animation slot. This causes
		the anim system to try and interrupt as many animations as possible.
	*/
	BOOL& mAllSlotsUsed = temple::GetRef<BOOL>(0x10AA4BB0);

	// Fixed size array of 512 slots
	AnimSlotArray& mSlots = temple::GetRef<AnimSlotArray>(0x118CE520);

	AnimGoalArray& mGoals = temple::GetRef<AnimGoalArray>(0x102BD1B0);

	void ProcessActionCallbacks();
	void PushActionCallback(AnimSlot &slot);

	bool PushGoal(AnimSlotGoalStackEntry *gdata, AnimSlotId* slotId);
	BOOL PushGoalImpl(AnimSlotGoalStackEntry *goalData, AnimSlotId * slotId, int a3, int flags);
	void PopGoal(AnimSlot & slot, uint32_t popFlags, const AnimGoal** newGoal, AnimSlotGoalStackEntry** newCurrentGoal, bool* stopProcessing);

	BOOL RescheduleEvent(int delayMs, AnimSlot &slot, const TimeEvent *oldEvt);

	void GoalDestinationsRemove(objHndl obj);

	bool Interrupt(objHndl actor, AnimGoalPriority priority, bool all);

	bool InterruptGoals(AnimSlot &slot, AnimGoalPriority priority);
	
	/*
		Will validate object references in the anim slot and set param1 and param2
		to whatever the given state requests. The state is optional. If it is null,
		only the object references will be validated.
	*/
	bool PrepareSlotForGoalState(AnimSlot &slot, const AnimGoalState *state);
	
	/*
		While processing the timer event for a slot, this will contain the slots index.
		Otherwise -1.
	*/
	int &mCurrentlyProcessingSlotIdx = temple::GetRef<int>(0x102B2654);

	std::vector<AnimActionCallback> mActionCallbacks;

	void DebugGoals();
	
};

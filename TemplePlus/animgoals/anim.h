
#pragma once
#include "obj.h"
#include "skill.h"
#include "gamesystems/gamesystem.h"
#include "animgoals/animgoal.h"
#include "animgoals.h"
#include <fmt/format.h>
#include <optional>

struct AnimPath;
struct AnimSlot;
struct AnimGoal;
struct AnimSlotGoalStackEntry;
struct AnimGoalState;
class AnimationGoals;
union AnimParam;

#pragma pack(push, 1)
struct AnimSlotId {
	int slotIndex;
	int uniqueId;
	int field_8;

	std::string ToString() const;
	void Clear() {
		slotIndex = -1;
		uniqueId = -1;
		field_8 = 0;
	}

	bool IsNull() const {
		return slotIndex == -1;
	}
};
#pragma pack(pop)

// Allows for direct use of AnimSlotId in format() strings
inline void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const AnimSlotId &id) {
	f.writer().write("{}", id.ToString());
}

struct PathQueryResult;

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

static constexpr int AnimSlotCount = 512;
using AnimSlotArray = AnimSlot[AnimSlotCount];

struct TimeEvent;
class AnimSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
friend class AnimSystemHooks;
friend class AnimationGoals;
friend class AnimGoalsDebugRenderer;
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
	int GetFirstRunSlotIdxForObj(objHndl handle) const;
	int GetNextRunSlotIdxForObj(objHndl handle, int startSlot) const;
	AnimSlot & GetRunSlot(int slotIdx) const;
	std::optional<AnimSlotId> GetFirstRunSlotId(objHndl handle) const;
	std::optional<AnimSlotId> GetRunSlotWithGoalWithoutFieldC(objHndl handle) const;
	std::optional<AnimSlotId> HasAttackAnim(objHndl handle, objHndl target = objHndl::null) const;

	bool DoesObjHaveGoal(objHndl handle, AnimGoalType goalType, AnimSlotId *idOut = nullptr) const;

	AnimGoalPriority GetCurrentPriority(objHndl handle) const;

	void SaveToMap(std::string_view mapName);

	bool ClearForObject(objHndl handle);

	void SetAllGoalsClearedCallback(std::function<void()> callback) {
		mAllGoalsClearedCallback = callback;
	}

	/*
		Interrupts currently running animation goals up to the given priority.
	*/
	bool Interrupt(objHndl actor, AnimGoalPriority priority, bool all = false);

	void InterruptGoalsByType(objHndl handle, AnimGoalType type, AnimGoalType keep);

	void InterruptAllGoalsUpToPriority(AnimGoalPriority priority);

	bool AddSubGoal(const AnimSlotId &id, const AnimSlotGoalStackEntry &stackEntry);

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

	
	void PushFallDown(objHndl actor, int unk);

	int PushAttackAnim(objHndl actor, objHndl target, int unk1, int hitAnimIdx, int playCrit, int useSecondaryAnim);
	int GetActionAnimId(objHndl objHndl);
	int PushAttemptAttack(objHndl attacker, objHndl defender);
	int PushGoalHitByWeapon(objHndl attacker, objHndl defender);
	int PushDodge(objHndl attacker, objHndl dodger);
	int PushAnimate(objHndl obj, int anim);

	/*
	pushes spell animation, including wand animation if relevant
	*/
	BOOL PushSpellCast(SpellPacketBody &spellPkt, objHndl item);
	BOOL PushSpellInterrupt(const objHndl& caster, objHndl item, AnimGoalType animGoalType, int spellSchool);

	/*
	 pushes an animation for dismissing an active spell
	 */
	BOOL PushSpellDismiss(SpellPacketBody &spellPkt);
	int GetWandAnimId(int school);
	int GetCastingAnimId(int school);

	/*
	used by the general out-of-combat mouse LMB click handler
	*/
	void PushForMouseTarget(objHndl handle, AnimGoalType type, objHndl tgt, locXY loc, objHndl scratchObj, int someFlag);

	void Debug();
	const AnimGoal* GetGoal(AnimGoalType goalType);
	int GetSlotUsedCount() const;

	void GoalDestinationRemove(objHndl);
	void GoalDestinationAdd(objHndl handle, LocAndOffsets loc);
	void SetRuninfoDeallocCallback(void(__cdecl* cb)());
	bool InterruptAllForTbCombat();

	/**
	 * Is the animation system currently processing an animation event?
	 */
	bool IsProcessing() const {
		return mCurrentlyProcessingSlotIdx != -1;
	}

	AnimSlotId GetLastSlotPushedTo() const {
		return lastSlotPushedTo_;
	}

	void NotifySpeedRecalc(objHndl parent); // sets ASF_SPEED_RECALC

private:
	friend class AnimGoalsHooks;
	std::unique_ptr<AnimationGoals> animationGoals_;

	std::optional<AnimSlotId> AllocSlot();
	BOOL GetSlot(AnimSlotId* runId, AnimSlot **runSlotOut);
	AnimSlot *GetSlot(const AnimSlotId &id);
	void FreeSlot(AnimSlot &slot);

	void SaveSlot(AnimSlot &slot, void* fh) const;
	void SaveGoalState(const AnimSlotGoalStackEntry &goal, void *fh) const;

	/*
		Set to true when ToEE cannot allocate an animation slot. This causes
		the anim system to try and interrupt as many animations as possible.
	*/
	BOOL& mAllSlotsUsed = temple::GetRef<BOOL>(0x10AA4BB0);
	uint32_t &mSlotsInUse = temple::GetRef<uint32_t>(0x10AA4BBC);

	// Fixed size array of 512 slots
	AnimSlotArray& mSlots = temple::GetRef<AnimSlotArray>(0x118CE520);

	AnimSlotId &animIdGlobal = temple::GetRef<AnimSlotId>(0x102AC880);

	int &animSysIsLoading = temple::GetRef<int>(0x10AA4BB8);

	// The last slot that a goal was pushed to
	AnimSlotId lastSlotPushedTo_{};
	
	void ProcessActionCallbacks();
	void PushActionCallback(AnimSlot &slot);

	bool PushGoal(const AnimSlotGoalStackEntry &gdata, AnimSlotId* slotId);
	bool PushGoalInternal(const AnimSlotGoalStackEntry &goalData, AnimSlotId * slotIdOut, int flags);

	bool PushFidgetInternal(objHndl handle);
	bool CritterCanAnimate(objHndl handle) const;
	
	void PopGoal(AnimSlot & slot, uint32_t popFlags, const AnimGoal** newGoal, AnimSlotGoalStackEntry** newCurrentGoal, bool* stopProcessing);

	BOOL RescheduleEvent(int delayMs, AnimSlot &slot, const TimeEvent *oldEvt);

	void GoalDestinationsRemove(objHndl obj);

	bool InterruptGoals(AnimSlot &slot, AnimGoalPriority priority);

	bool CurrentGoalHasField10_1(const AnimSlot &slot) const;
	
	/*
		Will validate object references in the anim slot and set param1 and param2
		to whatever the given state requests. The state is optional. If it is null,
		only the object references will be validated.
	*/
	bool PrepareSlotForGoalState(AnimSlot &slot, const AnimGoalState *state);
	AnimParam GetAnimParam(const AnimSlot &slot, AnimGoalProperty property) const;

	/**
	 * Find the anim slot for the given obj that has a goal of the same (or related) type running
	 * for the same object arguments.
	 */
	std::optional<AnimSlotId> GetSlotForGoalAndObjs(objHndl handle, const AnimSlotGoalStackEntry &goalData) const;
	
	bool IsEquivalentGoalType(AnimGoalType expected, AnimGoalType actual) const;

	/*
		While processing the timer event for a slot, this will contain the slots index.
		Otherwise -1.
	*/
	int &mCurrentlyProcessingSlotIdx = temple::GetRef<int>(0x102B2654);

	std::vector<AnimActionCallback> mActionCallbacks;

	void DebugGoals();
		
	void IncreaseActiveGoalCount(const AnimSlot &slot, const AnimGoal &goal);
	void DecreaseActiveGoalCount(const AnimSlot &slot, const AnimGoal &goal);
	int &mActiveGoalCount = temple::GetRef<int>(0x10AA4BC0);

	std::function<void()> mAllGoalsClearedCallback;
	
};


#include "stdafx.h"
#include <temple/dll.h>
#include <fmt/format.h>

#include "animgoals.h"
#include "animgoals_callbacks.h"

static const char *animGoalTypeNames[ag_count] = {
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

std::string_view GetAnimGoalTypeName(AnimGoalType type) {
	static auto sUnknownGoalType = "unknown_goal_type"sv;

	auto i = (size_t)type;
	if (type >= 0 && type < ag_count) {
		return animGoalTypeNames[type];
	}
	return sUnknownGoalType;
}

void format_arg(fmt::BasicFormatter<char>& f, const char *& format_str, const AnimGoalType & id)
{
	f.format(GetAnimGoalTypeName(id).data());
}

using GoalCallback = int(__cdecl *)(AnimSlot&);

class AnimGoalStateBuilder {
public:
	AnimGoalStateBuilder(AnimGoalState &state): state_(state) {
	}

	AnimGoalStateBuilder &SetArgs(AnimGoalProperty param1, AnimGoalProperty param2 = (AnimGoalProperty) -1) {
		state_.argInfo1 = param1;
		state_.argInfo2 = param2;
		return *this;
	}

	AnimGoalStateBuilder &SetFlagsData(int flagsData) {
		state_.flagsData = flagsData;
		return *this;
	}

	AnimGoalStateBuilder &OnSuccess(int transition, int delay = 0) {
		//Expects(transition != 0);
		state_.afterSuccess.newState = transition;
		state_.afterSuccess.delay = delay;
		return *this;
	}

	AnimGoalStateBuilder &OnFailure(int transition, int delay = 0) {
		//Expects(transition != 0);
		state_.afterFailure.newState = transition;
		state_.afterFailure.delay = delay;
		return *this;
	}

private:
	AnimGoalState & state_;
};

class AnimGoalCleanupBuilder {
public:
	AnimGoalCleanupBuilder(AnimGoalState &state) : state_(state) {
	}

	AnimGoalCleanupBuilder &SetArgs(AnimGoalProperty param1, AnimGoalProperty param2 = (AnimGoalProperty)-1) {
		state_.argInfo1 = param1;
		state_.argInfo2 = param2;
		return *this;
	}

	AnimGoalCleanupBuilder &SetFlagsData(int flagsData) {
		state_.flagsData = flagsData;
		return *this;
	}

private:
	AnimGoalState & state_;
};

class AnimGoalBuilder {
public:
	AnimGoalBuilder(AnimGoal &goal) : goal_(goal) {
	}

	AnimGoalBuilder &SetPriority(AnimGoalPriority priority) {
		goal_.priority = priority;
		return *this;
	}

	AnimGoalBuilder &SetFieldC(bool enable) {
		goal_.field_C = enable ? 1 : 0;
		return *this;
	}

	AnimGoalBuilder &SetField10(bool enable) {
		goal_.field_10 = enable ? 1 : 0;
		return *this;
	}

	AnimGoalBuilder &SetInterruptAll(bool enable) {
		goal_.interruptAll = enable;
		return *this;
	}

	AnimGoalCleanupBuilder AddCleanup(GoalCallback callback) {
		goal_.state_special.callback = callback;
		return AnimGoalCleanupBuilder(goal_.state_special);
	}

	AnimGoalStateBuilder AddState(GoalCallback callback) {
		auto &state = goal_.states[goal_.statecount++];
		state.callback = callback;
		return AnimGoalStateBuilder(state);
	}

	AnimGoalBuilder &SetRelatedGoals(AnimGoalType type1, AnimGoalType type2 = (AnimGoalType)-1, AnimGoalType type3 = (AnimGoalType) -1) {
		goal_.relatedGoal[0] = type1;
		goal_.relatedGoal[1] = type2;
		goal_.relatedGoal[2] = type3;
		return *this;
	}

private:
	AnimGoal & goal_;

};

static constexpr int T_INVALIDATE_PATH = ASTF_GOAL_INVALIDATE_PATH;
static constexpr int T_POP_GOAL = ASTF_POP_GOAL;
static constexpr int T_POP_GOAL_TWICE = ASTF_POP_GOAL_TWICE;
static constexpr int T_POP_ALL = ASTF_POP_ALL;
static constexpr int T_REWIND = ASTF_REWIND;
static constexpr int T_GOTO_STATE(int nextState) {
	return nextState + 1;
}
static constexpr int T_PUSH_GOAL(AnimGoalType goalType) {
	return ((int)ASTF_PUSH_GOAL) | ((int) goalType);
}

static constexpr int DELAY_CUSTOM = AnimStateTransition::DelayCustom;
static constexpr int DELAY_SLOT = AnimStateTransition::DelaySlot;
static constexpr int DELAY_RANDOM = AnimStateTransition::DelayRandom;

AnimationGoals::AnimationGoals()
{
	// ag_animate
	auto animate = AnimGoalBuilder(goals_[ag_animate])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	animate.AddCleanup(GoalAnimateCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	animate.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	animate.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	animate.AddState(GoalThrowItemPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(4));
	animate.AddState(GoalActionPerform3) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_animate_loop
	auto animate_loop = AnimGoalBuilder(goals_[ag_animate_loop])
		.SetPriority(AGP_7)
		.SetField10(true);
	animate_loop.AddCleanup(GoalFreeSoundHandle)
		.SetFlagsData(1);
	animate_loop.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(1));
	animate_loop.AddState(GoalLoopWhileCloseToParty) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_REWIND, 800);
	animate_loop.AddState(GoalAnimateFireDmgContinueAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_REWIND, DELAY_SLOT);
	animate_loop.AddState(GoalAnimateForever) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);

	// ag_anim_idle
	auto anim_idle = AnimGoalBuilder(goals_[ag_anim_idle])
		.SetPriority(AGP_7);
	anim_idle.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	anim_idle.AddState(GoalContinueWithAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(3));
	anim_idle.AddState(GoalStartIdleAnimIfCloseToParty) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	anim_idle.AddState(AlwaysSucceed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);

	// ag_anim_fidget
	auto anim_fidget = AnimGoalBuilder(goals_[ag_anim_fidget])
		.SetPriority(AGP_1);
	anim_fidget.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	anim_fidget.AddState(GoalContinueWithAnim2) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(4));
	anim_fidget.AddState(GoalStartFidgetAnimIfCloseToParty) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(3));
	anim_fidget.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	anim_fidget.AddState(GoalCritterShouldNotAutoAnimate) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);

	// ag_move_to_tile
	auto move_to_tile = AnimGoalBuilder(goals_[ag_move_to_tile])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_move_near_tile, ag_run_to_tile);
	move_to_tile.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	move_to_tile.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	move_to_tile.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	move_to_tile.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	move_to_tile.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(5));
	move_to_tile.AddState(GoalIsRotatedTowardNextPathNode) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	move_to_tile.AddState(GoalCalcPathToTarget) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(6));
	move_to_tile.AddState(AlwaysFail) // Index 6
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_run_to_tile
	auto run_to_tile = AnimGoalBuilder(goals_[ag_run_to_tile])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_move_near_tile, ag_move_to_tile, ag_run_near_tile);
	run_to_tile.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	run_to_tile.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	run_to_tile.AddState(GoalParam1ObjCloseToParam2Loc) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(2));
	run_to_tile.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	run_to_tile.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	run_to_tile.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(6));
	run_to_tile.AddState(GoalIsRotatedTowardNextPathNode) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	run_to_tile.AddState(GoalCalcPathToTarget) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	run_to_tile.AddState(AlwaysFail) // Index 7
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_attempt_move
	auto attempt_move = AnimGoalBuilder(goals_[ag_attempt_move])
		.SetPriority(AGP_2);
	attempt_move.AddCleanup(GoalAttemptMoveCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_move.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move.AddState(GoalHasDoorInPath) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(3));
	attempt_move.AddState(GoalIsDoorFullyClosed) // Index 2
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(3));
	attempt_move.AddState(GoalMoveAlongPath) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	attempt_move.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(9));
	attempt_move.AddState(GoalStateCallback7) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(6));
	attempt_move.AddState(GoalPlayMoveAnim) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);
	attempt_move.AddState(GoalPlayAnim) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_PUSH_GOAL(ag_open_door) | T_REWIND)
		.OnFailure(T_POP_ALL);
	attempt_move.AddState(AlwaysSucceed) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));
	attempt_move.AddState(AlwaysFail) // Index 9
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);
	attempt_move.AddState(0x0) // Index 10
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(0)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));
	attempt_move.AddState(0x0) // Index 11
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(0)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));

	// ag_move_to_pause
	auto move_to_pause = AnimGoalBuilder(goals_[ag_move_to_pause])
		.SetPriority(AGP_2)
		.SetFieldC(true);
	move_to_pause.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL, 1000)
		.OnFailure(T_POP_GOAL, 1000);
	move_to_pause.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_BLOCK_OBJ)
		.OnSuccess(T_POP_ALL, 1000)
		.OnFailure(T_POP_ALL);
	move_to_pause.AddState(AlwaysSucceed) // Index 2
		.SetArgs(AGDATA_BLOCK_OBJ)
		.OnSuccess(T_POP_ALL, 1000)
		.OnFailure(T_POP_ALL);

	// ag_move_near_tile
	auto move_near_tile = AnimGoalBuilder(goals_[ag_move_near_tile])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_run_to_tile, ag_move_to_tile, ag_run_near_tile);
	move_near_tile.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	move_near_tile.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	move_near_tile.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	move_near_tile.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	move_near_tile.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(5));
	move_near_tile.AddState(GoalIsRotatedTowardNextPathNode) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	move_near_tile.AddState(GoalFindPathNear) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(6));
	move_near_tile.AddState(AlwaysFail) // Index 6
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_move_near_obj
	auto move_near_obj = AnimGoalBuilder(goals_[ag_move_near_obj])
		.SetPriority(AGP_2);
	move_near_obj.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	move_near_obj.AddState(GoalIsTargetWithinRadius) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	move_near_obj.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	move_near_obj.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	move_near_obj.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_near))
		.OnFailure(T_GOTO_STATE(4));
	move_near_obj.AddState(GoalFindPathNearObject) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_near))
		.OnFailure(T_GOTO_STATE(5));
	move_near_obj.AddState(AlwaysFail) // Index 5
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_move_straight
	auto move_straight = AnimGoalBuilder(goals_[ag_move_straight])
		.SetPriority(AGP_3);
	move_straight.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	move_straight.AddState(GoalIsCurrentPathValid) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_straight))
		.OnFailure(T_GOTO_STATE(2));
	move_straight.AddState(GoalCalcPathToTarget2) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_straight))
		.OnFailure(T_GOTO_STATE(3));
	move_straight.AddState(AlwaysFail) // Index 3
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_attempt_move_straight
	auto attempt_move_straight = AnimGoalBuilder(goals_[ag_attempt_move_straight])
		.SetPriority(AGP_3);
	attempt_move_straight.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move_straight.AddState(GoalContinueMoveStraight) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_CUSTOM)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	attempt_move_straight.AddState(GoalPlayWaterRipples) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);

	// ag_open_door
	auto open_door = AnimGoalBuilder(goals_[ag_open_door])
		.SetPriority(AGP_3)
		.SetInterruptAll(true)
		.SetFieldC(true);
	open_door.AddState(GoalIsParam1Door) // Index 0
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	open_door.AddState(GoalIsDoorFullyClosed) // Index 1
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_GOAL);
	open_door.AddState(GoalAttemptOpenDoor) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	open_door.AddState(GoalIsDoorLocked) // Index 3
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(5));
	open_door.AddState(GoalDoorAlwaysFalse) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_unlock_door))
		.OnFailure(T_POP_ALL);
	open_door.AddState(GoalUseObject) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	open_door.AddState(GoalPlayDoorLockedSound) // Index 6
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_POP_ALL);
	open_door.AddState(GoalStateCallback1) // Index 7
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(66)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);

	// ag_attempt_open_door
	auto attempt_open_door = AnimGoalBuilder(goals_[ag_attempt_open_door])
		.SetPriority(AGP_3)
		.SetInterruptAll(true)
		.SetFieldC(true);
	attempt_open_door.AddState(GoalIsParam1Door) // Index 0
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	attempt_open_door.AddState(GoalIsDoorFullyClosed) // Index 1
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);
	attempt_open_door.AddState(GoalAttemptOpenDoor) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	attempt_open_door.AddState(GoalIsDoorLocked) // Index 3
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(5));
	attempt_open_door.AddState(GoalDoorAlwaysFalse) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_unlock_door))
		.OnFailure(T_POP_ALL);
	attempt_open_door.AddState(GoalUseObject) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	attempt_open_door.AddState(GoalPlayDoorLockedSound) // Index 6
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_POP_ALL);
	attempt_open_door.AddState(GoalStateCallback1) // Index 7
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(66)
		.OnSuccess(T_POP_GOAL | 0x1000000)
		.OnFailure(T_POP_ALL);


	// ag_unlock_door was undefined


	// ag_jump_window was undefined


	// ag_pickup_item was undefined


	// ag_attempt_pickup was undefined

	// ag_pickpocket
	auto pickpocket = AnimGoalBuilder(goals_[ag_pickpocket])
		.SetPriority(AGP_3);
	pickpocket.AddState(GoalSaveStateDataInSkillData) // Index 0
		.SetFlagsData(12)
		.OnSuccess(T_PUSH_GOAL(ag_use_skill_on))
		.OnFailure(T_POP_ALL);

	// ag_attack
	auto attack = AnimGoalBuilder(goals_[ag_attack])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_attempt_attack);
	attack.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attack.AddState(GoalIsAlive) // Index 0
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	attack.AddState(GoalAttackEndTurnIfUnreachable) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);
	attack.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	attack.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	attack.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_attack) | 0x4000000)
		.OnFailure(T_GOTO_STATE(5));
	attack.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 5
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	attack.AddState(GoalIsTargetWithinRadius) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj_combat));
	attack.AddState(AlwaysFail) // Index 7
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_attempt_attack
	auto attempt_attack = AnimGoalBuilder(goals_[ag_attempt_attack])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_attack);
	attempt_attack.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_attack.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_attack.AddState(GoalAttackContinueWithAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(8));
	attempt_attack.AddState(GoalSetRotationToFaceTargetObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(9));
	attempt_attack.AddState(AlwaysSucceed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(11));
	attempt_attack.AddState(GoalEnterCombat) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(9));
	attempt_attack.AddState(GoalAttackPlayWeaponHitEffect) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(9));
	attempt_attack.AddState(GoalSlotFlagSet8If4AndNotSetYet) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(12))
		.OnFailure(T_REWIND, DELAY_SLOT);
	attempt_attack.AddState(GoalActionPerform2) // Index 7
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(9));
	attempt_attack.AddState(GoalHasReachWithMainWeapon) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(9))
		.OnFailure(T_GOTO_STATE(9));
	attempt_attack.AddState(GoalPlayAnim) // Index 9
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(10))
		.OnFailure(T_GOTO_STATE(10));
	attempt_attack.AddState(GoalAttackPlayIdleAnim) // Index 10
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(11))
		.OnFailure(T_GOTO_STATE(11));
	attempt_attack.AddState(GoalAttemptAttackCheck) // Index 11
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attack) | 0x4000000, 5)
		.OnFailure(T_POP_GOAL);
	attempt_attack.AddState(AlwaysSucceed) // Index 12
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(11));

	// ag_talk
	auto talk = AnimGoalBuilder(goals_[ag_talk])
		.SetPriority(AGP_3);
	talk.AddState(GoalNotPreventedFromTalking) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	talk.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	talk.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	talk.AddState(GoalIsWithinTalkingDistance) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(5));
	talk.AddState(GoalInitiateDialog) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	talk.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 5
		.SetFlagsData(5)
		.OnSuccess(T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_POP_ALL);


	// ag_pick_weapon was undefined

	// ag_chase
	auto chase = AnimGoalBuilder(goals_[ag_chase])
		.SetPriority(AGP_3);
	chase.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	chase.AddState(GoalSetRadiusToAiSpread) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);
	chase.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj));
	chase.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_REWIND, 100)
		.OnFailure(T_POP_ALL);

	// ag_follow
	auto follow = AnimGoalBuilder(goals_[ag_follow])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_run_near_obj, ag_move_near_obj);
	follow.AddState(GoalSetRadiusToAiSpread) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	follow.AddState(GoalIsCloserThanDesiredSpread) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(2));
	follow.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(3));
	follow.AddState(AlwaysFail) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_run_near_obj));
	follow.AddState(GoalActionPerform3) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_REWIND, 100)
		.OnFailure(T_POP_ALL);

	// ag_follow_to_location
	auto follow_to_location = AnimGoalBuilder(goals_[ag_follow_to_location])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_run_near_obj, ag_move_near_obj);
	follow_to_location.AddState(GoalSetRadiusTo4) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	follow_to_location.AddState(GoalTargetLocWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_run_to_tile))
		.OnFailure(T_PUSH_GOAL(ag_follow));

	// ag_flee
	auto flee = AnimGoalBuilder(goals_[ag_flee])
		.SetPriority(AGP_3);
	flee.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	flee.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 1
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);
	flee.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_move_away_from_obj))
		.OnFailure(T_GOTO_STATE(3));
	flee.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_GOAL, DELAY_SLOT)
		.OnFailure(T_POP_ALL);

	// ag_throw_spell
	auto throw_spell = AnimGoalBuilder(goals_[ag_throw_spell])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_attempt_spell);
	throw_spell.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	throw_spell.AddState(AlwaysSucceed) // Index 0
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	throw_spell.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	throw_spell.AddState(GoalReturnTrue) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(7));
	throw_spell.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_PUSH_GOAL(ag_pick_weapon));
	throw_spell.AddState(AlwaysSucceed) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(8))
		.OnFailure(T_GOTO_STATE(6));
	throw_spell.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 6
		.SetFlagsData(8)
		.OnSuccess(T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell.AddState(AlwaysSucceed) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(9))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell.AddState(GoalSetRotationToFaceTargetObj) // Index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_spell))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell.AddState(GoalCastConjureEnd) // Index 9
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_attempt_spell
	auto attempt_spell = AnimGoalBuilder(goals_[ag_attempt_spell])
		.SetPriority(AGP_4)
		.SetRelatedGoals(ag_throw_spell);
	attempt_spell.AddCleanup(GoalCastConjureEnd)
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1);
	attempt_spell.AddState(GoalIsNotStackFlagsData40) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(10))
		.OnFailure(T_GOTO_STATE(1));
	attempt_spell.AddState(GoalSetSlotFlags4) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(7), DELAY_SLOT);
	attempt_spell.AddState(AlwaysSucceed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(11));
	attempt_spell.AddState(GoalSlotFlagSet8If4AndNotSetYet) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_REWIND, DELAY_SLOT);
	attempt_spell.AddState(GoalReturnFalse) // Index 4
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(8));
	attempt_spell.AddState(GoalSpawnFireball) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, SELF_OBJ_PRECISE_LOC)
		.SetFlagsData(5)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(7));
	attempt_spell.AddState(GoalStateCallback1) // Index 6
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(29)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL, DELAY_SLOT);
	attempt_spell.AddState(AlwaysSucceed) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	attempt_spell.AddState(GoalAttemptSpell) // Index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	attempt_spell.AddState(GoalActionPerform3) // Index 9
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	attempt_spell.AddState(AlwaysSucceed) // Index 10
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(11));
	attempt_spell.AddState(GoalAttemptSpell) // Index 11
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_shoot_spell
	auto shoot_spell = AnimGoalBuilder(goals_[ag_shoot_spell])
		.SetPriority(AGP_3);
	shoot_spell.AddCleanup(GoalDestroyParam1)
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1);
	shoot_spell.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, TARGET_LOC_PRECISE)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(1));
	shoot_spell.AddState(GoalIsCurrentPathValid) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(2));
	shoot_spell.AddState(GoalCalcPathToTarget2) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, TARGET_LOC_PRECISE)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_POP_ALL);
	shoot_spell.AddState(GoalAreOnSameTile) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(8));
	shoot_spell.AddState(GoalAttemptSpell) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_POP_ALL);
	shoot_spell.AddState(GoalSetOffAndDestroyParam1) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	shoot_spell.AddState(AlwaysFail) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(5));
	shoot_spell.AddState(AlwaysFail) // Index 7
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_straight_spell))
		.OnFailure(T_POP_ALL);
	shoot_spell.AddState(GoalSetTargetLocFromObj) // Index 8
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(0))
		.OnFailure(T_POP_GOAL);

	// ag_hit_by_spell
	auto hit_by_spell = AnimGoalBuilder(goals_[ag_hit_by_spell])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	hit_by_spell.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	hit_by_spell.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	hit_by_spell.AddState(AlwaysFail) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SPELL_DATA)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(3));
	hit_by_spell.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_hit_by_weapon
	auto hit_by_weapon = AnimGoalBuilder(goals_[ag_hit_by_weapon])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	hit_by_weapon.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	hit_by_weapon.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL, DELAY_SLOT);
	hit_by_weapon.AddState(GoalPlayGetHitAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	hit_by_weapon.AddState(GoalThrowItemPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);

	// ag_dodge
	auto dodge = AnimGoalBuilder(goals_[ag_dodge])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	dodge.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	dodge.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL, DELAY_SLOT);
	dodge.AddState(GoalPlayDodgeAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	dodge.AddState(GoalThrowItemPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);

	// ag_dying
	auto dying = AnimGoalBuilder(goals_[ag_dying])
		.SetPriority(AGP_5)
		.SetFieldC(true);
	dying.AddCleanup(GoalDyingCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	dying.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	dying.AddState(GoalDyingContinueAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(6));
	dying.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(3));
	dying.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(4));
	dying.AddState(GoalLeaveCombat) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(6));
	dying.AddState(GoalDyingPlaySoundAndRipples) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(6));
	dying.AddState(GoalDyingReturnTrue) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(8))
		.OnFailure(T_GOTO_STATE(7));
	dying.AddState(GoalActionPerform3) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	dying.AddState(GoalSetNoBlockIfNotInParty) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_destroy_obj
	auto destroy_obj = AnimGoalBuilder(goals_[ag_destroy_obj])
		.SetPriority(AGP_5);
	destroy_obj.AddState(GoalSetOffAndDestroyParam1) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_skill_on
	auto use_skill_on = AnimGoalBuilder(goals_[ag_use_skill_on])
		.SetPriority(AGP_3);
	use_skill_on.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	use_skill_on.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 0
		.SetFlagsData(5)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_skill_on.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(2));
	use_skill_on.AddState(GoalIsCurrentPathValid) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(3));
	use_skill_on.AddState(GoalFindPathNearObject) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(6));
	use_skill_on.AddState(GoalIsRotatedTowardTarget) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	use_skill_on.AddState(GoalCheckParam2AgainstStateFlagData) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_FLAGS_DATA)
		.SetFlagsData(11)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_use_picklock_skill_on))
		.OnFailure(T_GOTO_STATE(7));
	use_skill_on.AddState(AlwaysFail) // Index 6
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);
	use_skill_on.AddState(GoalCheckParam2AgainstStateFlagData) // Index 7
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_FLAGS_DATA)
		.SetFlagsData(12)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_use_pickpocket_skill_on))
		.OnFailure(T_GOTO_STATE(8));
	use_skill_on.AddState(GoalCheckParam2AgainstStateFlagData) // Index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_FLAGS_DATA)
		.SetFlagsData(4)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_use_disable_device_skill_on_data))
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_use_skill_on));

	// ag_attempt_use_skill_on
	auto attempt_use_skill_on = AnimGoalBuilder(goals_[ag_attempt_use_skill_on])
		.SetPriority(AGP_3);
	attempt_use_skill_on.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_use_skill_on.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_use_skill_on.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(7), DELAY_SLOT);
	attempt_use_skill_on.AddState(GoalSetRotationToFaceTargetObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	attempt_use_skill_on.AddState(GoalPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(111)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_ALL);
	attempt_use_skill_on.AddState(GoalThrowItemPlayAnim) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(8));
	attempt_use_skill_on.AddState(GoalSlotFlagSet8If4AndNotSetYet) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_REWIND, DELAY_SLOT);
	attempt_use_skill_on.AddState(GoalActionPerform) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	attempt_use_skill_on.AddState(GoalPlayAnim) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	attempt_use_skill_on.AddState(GoalActionPerform3) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);


	// ag_skill_conceal was undefined

	// ag_projectile
	auto projectile = AnimGoalBuilder(goals_[ag_projectile])
		.SetPriority(AGP_5);
	projectile.AddCleanup(GoalProjectileCleanup)
		.SetFlagsData(1);
	projectile.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	projectile.AddState(GoalUpdateMoveStraight) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_REWIND, DELAY_CUSTOM)
		.OnFailure(T_POP_GOAL);
	projectile.AddState(GoalParam1ObjCloseToParam2Loc) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(3));
	projectile.AddState(GoalStateCallback3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_GOAL);
	projectile.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(5));
	projectile.AddState(AlwaysSucceed) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_GOAL);
	projectile.AddState(GoalBeginMoveStraight) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_GOAL);

	// ag_throw_item
	auto throw_item = AnimGoalBuilder(goals_[ag_throw_item])
		.SetPriority(AGP_3);
	throw_item.AddCleanup(GoalThrowItemCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	throw_item.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	throw_item.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(7), DELAY_SLOT);
	throw_item.AddState(GoalPlayAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(7));
	throw_item.AddState(GoalSetRotationToFaceTargetLoc) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(7));
	throw_item.AddState(GoalThrowItemPlayAnim) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	throw_item.AddState(GoalSlotFlagSet8If4AndNotSetYet) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_REWIND, DELAY_SLOT);
	throw_item.AddState(GoalThrowItem) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.SetFlagsData(5)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(7));
	throw_item.AddState(GoalPlayAnim) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(8))
		.OnFailure(T_GOTO_STATE(8));
	throw_item.AddState(GoalActionPerform3) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_object
	auto use_object = AnimGoalBuilder(goals_[ag_use_object])
		.SetPriority(AGP_3);
	use_object.AddState(GoalSetRadiusTo2) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_object.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj));
	use_object.AddState(GoalIsRotatedTowardTarget) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	use_object.AddState(GoalIsParam1Door) // Index 3
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(4));
	use_object.AddState(GoalUseObject) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	use_object.AddState(GoalSaveParam1InScratch) // Index 5
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	use_object.AddState(GoalIsDoorFullyClosed) // Index 6
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_open_door))
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_close_door));

	// ag_use_item_on_object
	auto use_item_on_object = AnimGoalBuilder(goals_[ag_use_item_on_object])
		.SetPriority(AGP_3);
	use_item_on_object.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 0
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_item_on_object.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj));
	use_item_on_object.AddState(GoalUseItemOnObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_item_on_object_with_skill
	auto use_item_on_object_with_skill = AnimGoalBuilder(goals_[ag_use_item_on_object_with_skill])
		.SetPriority(AGP_3);
	use_item_on_object_with_skill.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 0
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_item_on_object_with_skill.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj));
	use_item_on_object_with_skill.AddState(GoalUseItemOnObjWithSkillDummy) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_item_on_tile
	auto use_item_on_tile = AnimGoalBuilder(goals_[ag_use_item_on_tile])
		.SetPriority(AGP_3);
	use_item_on_tile.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 0
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_item_on_tile.AddState(GoalTargetLocWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_tile));
	use_item_on_tile.AddState(GoalUseItemOnLoc) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_item_on_tile_with_skill
	auto use_item_on_tile_with_skill = AnimGoalBuilder(goals_[ag_use_item_on_tile_with_skill])
		.SetPriority(AGP_3);
	use_item_on_tile_with_skill.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 0
		.SetFlagsData(1)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_item_on_tile_with_skill.AddState(GoalTargetLocWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_tile));
	use_item_on_tile_with_skill.AddState(GoalUseItemOnLocWithSkillDummy) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_knockback
	auto knockback = AnimGoalBuilder(goals_[ag_knockback])
		.SetPriority(AGP_5);
	knockback.AddState(GoalParam1ObjCloseToParam2Loc) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	knockback.AddState(GoalIsCurrentPathValid) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_straight_knockback))
		.OnFailure(T_GOTO_STATE(2));
	knockback.AddState(GoalKnockbackFunc) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_straight_knockback))
		.OnFailure(T_GOTO_STATE(3));
	knockback.AddState(AlwaysSucceed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_floating
	auto floating = AnimGoalBuilder(goals_[ag_floating])
		.SetPriority(AGP_5)
		.SetInterruptAll(true)
		.SetFieldC(true)
		.SetField10(true);
	floating.AddState(GoalIsNotStackFlagsData20) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	floating.AddState(GoalJiggleAlongYAxis) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, 100)
		.OnFailure(T_GOTO_STATE(3), 100);
	floating.AddState(GoalStartJigglingAlongYAxis) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);
	floating.AddState(GoalEndJigglingAlongYAxis) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_close_door
	auto close_door = AnimGoalBuilder(goals_[ag_close_door])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	close_door.AddState(GoalIsParam1Door) // Index 0
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	close_door.AddState(GoalIsDoorFullyClosed) // Index 1
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_close_door));

	// ag_attempt_close_door
	auto attempt_close_door = AnimGoalBuilder(goals_[ag_attempt_close_door])
		.SetPriority(AGP_3)
		.SetInterruptAll(true);
	attempt_close_door.AddState(GoalIsParam1Door) // Index 0
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	attempt_close_door.AddState(GoalIsDoorFullyClosed) // Index 1
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	attempt_close_door.AddState(GoalIsDoorMagicallyHeld) // Index 2
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	attempt_close_door.AddState(GoalStateCallback1) // Index 3
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(67)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);

	// ag_animate_reverse
	auto animate_reverse = AnimGoalBuilder(goals_[ag_animate_reverse])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	animate_reverse.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	animate_reverse.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate_reverse.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	animate_reverse.AddState(AlwaysFail) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(3));
	animate_reverse.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_move_away_from_obj
	auto move_away_from_obj = AnimGoalBuilder(goals_[ag_move_away_from_obj])
		.SetPriority(AGP_2);
	move_away_from_obj.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	move_away_from_obj.AddState(GoalIsProne) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(1));
	move_away_from_obj.AddState(GoalIsConcealed) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(2));
	move_away_from_obj.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_GOAL);
	move_away_from_obj.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(4));
	move_away_from_obj.AddState(GoalMoveAwayFromObj) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(5));
	move_away_from_obj.AddState(GoalTurnTowardsOrAway) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	move_away_from_obj.AddState(GoalSetNoFlee) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_rotate
	auto rotate = AnimGoalBuilder(goals_[ag_rotate])
		.SetPriority(AGP_2);
	rotate.AddCleanup(GoalSetRotationToParam2)
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_VAL2);
	rotate.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(2));
	rotate.AddState(GoalPlayRotationAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_VAL2)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_GOAL);
	rotate.AddState(GoalRotate) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_VAL2)
		.OnSuccess(T_REWIND, 15)
		.OnFailure(T_POP_GOAL);

	// ag_unconceal
	auto unconceal = AnimGoalBuilder(goals_[ag_unconceal])
		.SetPriority(AGP_4);
	unconceal.AddCleanup(GoalUnconcealCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	unconceal.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	unconceal.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	unconceal.AddState(GoalPlayUnconcealAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);
	unconceal.AddState(0x0) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(0)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));
	unconceal.AddState(0x0) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(0)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));

	// ag_run_near_tile
	auto run_near_tile = AnimGoalBuilder(goals_[ag_run_near_tile])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_move_near_tile, ag_run_to_tile);
	run_near_tile.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	run_near_tile.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	run_near_tile.AddState(GoalParam1ObjCloseToParam2Loc) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(2));
	run_near_tile.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	run_near_tile.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	run_near_tile.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(6));
	run_near_tile.AddState(GoalIsRotatedTowardNextPathNode) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	run_near_tile.AddState(GoalFindPathNear) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	run_near_tile.AddState(AlwaysFail) // Index 7
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_run_near_obj
	auto run_near_obj = AnimGoalBuilder(goals_[ag_run_near_obj])
		.SetPriority(AGP_2);
	run_near_obj.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	run_near_obj.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	run_near_obj.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(2));
	run_near_obj.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	run_near_obj.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	run_near_obj.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(5));
	run_near_obj.AddState(GoalFindPathNearObject) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_GOTO_STATE(6));
	run_near_obj.AddState(AlwaysFail) // Index 6
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_animate_stunned
	auto animate_stunned = AnimGoalBuilder(goals_[ag_animate_stunned])
		.SetPriority(AGP_1)
		.SetFieldC(true);
	animate_stunned.AddCleanup(GoalResetToIdleAnimUnstun)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	animate_stunned.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate_stunned.AddState(GoalStunnedContinueAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(6));
	animate_stunned.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	animate_stunned.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	animate_stunned.AddState(GoalStunnedPlayAnim) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(5));
	animate_stunned.AddState(GoalActionPerform3) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	animate_stunned.AddState(GoalStunnedExpire) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(4));

	// ag_animate_kneel_magic_hands
	auto animate_kneel_magic_hands = AnimGoalBuilder(goals_[ag_animate_kneel_magic_hands])
		.SetPriority(AGP_3);
	animate_kneel_magic_hands.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate_kneel_magic_hands.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_animate_reverse));
	animate_kneel_magic_hands.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	animate_kneel_magic_hands.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	animate_kneel_magic_hands.AddState(AlwaysFail) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(5));
	animate_kneel_magic_hands.AddState(GoalActionPerform3) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_attempt_move_near
	auto attempt_move_near = AnimGoalBuilder(goals_[ag_attempt_move_near])
		.SetPriority(AGP_2);
	attempt_move_near.AddCleanup(GoalAttemptMoveCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_move_near.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move_near.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(9), DELAY_RANDOM)
		.OnFailure(T_GOTO_STATE(2));
	attempt_move_near.AddState(GoalMoveAlongPath) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	attempt_move_near.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(11));
	attempt_move_near.AddState(GoalStateCallback8) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(5));
	attempt_move_near.AddState(AlwaysFail) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(10))
		.OnFailure(T_GOTO_STATE(6));
	attempt_move_near.AddState(GoalHasDoorInPath) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(8));
	attempt_move_near.AddState(GoalIsDoorFullyClosed) // Index 7
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_INVALIDATE_PATH | T_PUSH_GOAL(ag_open_door) | T_REWIND, 50)
		.OnFailure(T_GOTO_STATE(8));
	attempt_move_near.AddState(GoalPlayMoveAnim) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(9));
	attempt_move_near.AddState(GoalActionPerform3) // Index 9
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_GOAL);
	attempt_move_near.AddState(GoalIsDoorUnlocked) // Index 10
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_jump_window) | T_REWIND)
		.OnFailure(T_POP_ALL);
	attempt_move_near.AddState(AlwaysFail) // Index 11
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_knock_down
	auto knock_down = AnimGoalBuilder(goals_[ag_knock_down])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	knock_down.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	knock_down.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(3));
	knock_down.AddState(AlwaysFail) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);
	knock_down.AddState(GoalIsAliveAndConscious) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_anim_get_up), 200)
		.OnFailure(T_POP_GOAL);

	// ag_anim_get_up
	auto anim_get_up = AnimGoalBuilder(goals_[ag_anim_get_up])
		.SetPriority(AGP_3);
	anim_get_up.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	anim_get_up.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	anim_get_up.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL);
	anim_get_up.AddState(GoalPlayGetUpAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(3));
	anim_get_up.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_attempt_move_straight_knockback
	auto attempt_move_straight_knockback = AnimGoalBuilder(goals_[ag_attempt_move_straight_knockback])
		.SetPriority(AGP_3);
	attempt_move_straight_knockback.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move_straight_knockback.AddState(GoalApplyKnockback) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_CUSTOM)
		.OnFailure(T_POP_ALL);
	attempt_move_straight_knockback.AddState(GoalPlayWaterRipples) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);
	attempt_move_straight_knockback.AddState(0x0) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(0)
		.OnSuccess(T_GOTO_STATE(4294967295))
		.OnFailure(T_GOTO_STATE(4294967295));

	// ag_wander
	auto wander = AnimGoalBuilder(goals_[ag_wander])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_move_near_tile);
	wander.AddState(GoalWander) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_move_near_tile), 300)
		.OnFailure(T_REWIND, 300);

	// ag_wander_seek_darkness
	auto wander_seek_darkness = AnimGoalBuilder(goals_[ag_wander_seek_darkness])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_move_near_tile);
	wander_seek_darkness.AddState(GoalWanderSeekDarkness) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_move_near_tile), 300)
		.OnFailure(T_REWIND, 300);

	// ag_use_picklock_skill_on
	auto use_picklock_skill_on = AnimGoalBuilder(goals_[ag_use_picklock_skill_on])
		.SetPriority(AGP_3);
	use_picklock_skill_on.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	use_picklock_skill_on.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	use_picklock_skill_on.AddState(GoalPickLockContinueWithAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(5), DELAY_SLOT);
	use_picklock_skill_on.AddState(GoalSetRotationToFaceTargetObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	use_picklock_skill_on.AddState(GoalPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(70)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_ALL);
	use_picklock_skill_on.AddState(GoalPickLockPlayPushDoorOpenAnim) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(6));
	use_picklock_skill_on.AddState(GoalPickLock) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	use_picklock_skill_on.AddState(GoalPlayAnim) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);

	// ag_please_move
	auto please_move = AnimGoalBuilder(goals_[ag_please_move])
		.SetPriority(AGP_2)
		.SetRelatedGoals(ag_move_near_tile, ag_run_to_tile, ag_move_to_tile);
	please_move.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	please_move.AddState(GoalPleaseMove) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_GOTO_STATE(1), DELAY_CUSTOM)
		.OnFailure(T_POP_GOAL);
	please_move.AddState(GoalParam1ObjCloseToParam2Loc) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(2));
	please_move.AddState(GoalIsProne) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(3));
	please_move.AddState(GoalIsConcealed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(4));
	please_move.AddState(GoalIsCurrentPathValid) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(6));
	please_move.AddState(GoalIsRotatedTowardNextPathNode) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	please_move.AddState(GoalCalcPathToTarget) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	please_move.AddState(AlwaysFail) // Index 7
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_attempt_spread_out
	auto attempt_spread_out = AnimGoalBuilder(goals_[ag_attempt_spread_out])
		.SetPriority(AGP_2)
		.SetInterruptAll(true)
		.SetRelatedGoals(ag_run_near_obj, ag_move_near_obj);
	attempt_spread_out.AddState(GoalSetRadiusToAiSpread) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	attempt_spread_out.AddState(GoalIsCloserThanDesiredSpread) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_move_away_from_obj))
		.OnFailure(T_GOTO_STATE(2));
	attempt_spread_out.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(3));
	attempt_spread_out.AddState(AlwaysFail) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_run_near_obj));
	attempt_spread_out.AddState(GoalActionPerform3) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_REWIND, 100)
		.OnFailure(T_POP_ALL);

	// ag_animate_door_open
	auto animate_door_open = AnimGoalBuilder(goals_[ag_animate_door_open])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	animate_door_open.AddCleanup(GoalOpenDoorCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	animate_door_open.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate_door_open.AddState(GoalContinueWithDoorOpenAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(3));
	animate_door_open.AddState(GoalPlayDoorOpenAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL);
	animate_door_open.AddState(GoalIsDoorSticky) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_pend_closing_door), 1500)
		.OnFailure(T_POP_GOAL);

	// ag_animate_door_closed
	auto animate_door_closed = AnimGoalBuilder(goals_[ag_animate_door_closed])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	animate_door_closed.AddCleanup(GoalCloseDoorCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	animate_door_closed.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	animate_door_closed.AddState(GoalContinueWithDoorCloseAnim) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	animate_door_closed.AddState(GoalPlayDoorCloseAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL);

	// ag_pend_closing_door
	auto pend_closing_door = AnimGoalBuilder(goals_[ag_pend_closing_door])
		.SetPriority(AGP_3)
		.SetFieldC(true);
	pend_closing_door.AddState(GoalIsLiveCritterNear) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_animate_door_closed), 1500)
		.OnFailure(T_REWIND, 1500);

	// ag_throw_spell_friendly
	auto throw_spell_friendly = AnimGoalBuilder(goals_[ag_throw_spell_friendly])
		.SetPriority(AGP_3)
		.SetInterruptAll(true)
		.SetRelatedGoals(ag_attempt_spell);
	throw_spell_friendly.AddState(AlwaysSucceed) // Index 0
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_GOAL);
	throw_spell_friendly.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	throw_spell_friendly.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(3));
	throw_spell_friendly.AddState(GoalReturnTrue) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(7));
	throw_spell_friendly.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_PUSH_GOAL(ag_pick_weapon));
	throw_spell_friendly.AddState(AlwaysSucceed) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(8))
		.OnFailure(T_POP_ALL);
	throw_spell_friendly.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 6
		.SetFlagsData(8)
		.OnSuccess(T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_POP_ALL);
	throw_spell_friendly.AddState(AlwaysSucceed) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	throw_spell_friendly.AddState(AlwaysSucceed) // Index 8
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_spell))
		.OnFailure(T_POP_ALL);

	// ag_attempt_spell_friendly
	auto attempt_spell_friendly = AnimGoalBuilder(goals_[ag_attempt_spell_friendly])
		.SetPriority(AGP_4)
		.SetInterruptAll(true)
		.SetRelatedGoals(ag_throw_spell);
	attempt_spell_friendly.AddCleanup(GoalCastConjureEnd)
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1);
	attempt_spell_friendly.AddState(GoalIsNotStackFlagsData40) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(10))
		.OnFailure(T_GOTO_STATE(1));
	attempt_spell_friendly.AddState(GoalSetSlotFlags4) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(7), DELAY_SLOT);
	attempt_spell_friendly.AddState(AlwaysSucceed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(11));
	attempt_spell_friendly.AddState(GoalSlotFlagSet8If4AndNotSetYet) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_REWIND, DELAY_SLOT);
	attempt_spell_friendly.AddState(GoalReturnFalse) // Index 4
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(8));
	attempt_spell_friendly.AddState(GoalSpawnFireball) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, SELF_OBJ_PRECISE_LOC)
		.SetFlagsData(5)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(7));
	attempt_spell_friendly.AddState(GoalStateCallback1) // Index 6
		.SetArgs(AGDATA_SCRATCH_OBJ, AGDATA_SELF_OBJ)
		.SetFlagsData(29)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL, DELAY_SLOT);
	attempt_spell_friendly.AddState(AlwaysSucceed) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	attempt_spell_friendly.AddState(GoalAttemptSpell) // Index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(7));
	attempt_spell_friendly.AddState(AlwaysSucceed) // Index 9
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	attempt_spell_friendly.AddState(AlwaysSucceed) // Index 10
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(11));
	attempt_spell_friendly.AddState(GoalAttemptSpell) // Index 11
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_animate_loop_fire_dmg
	auto animate_loop_fire_dmg = AnimGoalBuilder(goals_[ag_animate_loop_fire_dmg])
		.SetPriority(AGP_1)
		.SetFieldC(true)
		.SetField10(true);
	animate_loop_fire_dmg.AddCleanup(GoalFreeSoundHandle)
		.SetFlagsData(1);
	animate_loop_fire_dmg.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(1));
	animate_loop_fire_dmg.AddState(GoalLoopWhileCloseToParty) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_REWIND, 800);
	animate_loop_fire_dmg.AddState(GoalAnimateFireDmgContinueAnim) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(4));
	animate_loop_fire_dmg.AddState(GoalAnimateForever) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_ALL);
	animate_loop_fire_dmg.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_PARENT_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);

	// ag_attempt_move_straight_spell
	auto attempt_move_straight_spell = AnimGoalBuilder(goals_[ag_attempt_move_straight_spell])
		.SetPriority(AGP_3);
	attempt_move_straight_spell.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move_straight_spell.AddState(GoalContinueMoveStraight) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_CUSTOM)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	attempt_move_straight_spell.AddState(GoalPlayWaterRipples) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_ALL);

	// ag_move_near_obj_combat
	auto move_near_obj_combat = AnimGoalBuilder(goals_[ag_move_near_obj_combat])
		.SetPriority(AGP_2);
	move_near_obj_combat.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	move_near_obj_combat.AddState(GoalIsTargetWithinRadius) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(1));
	move_near_obj_combat.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	move_near_obj_combat.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	move_near_obj_combat.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(4));
	move_near_obj_combat.AddState(GoalFindPathNearObjectCombat) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_GOTO_STATE(5));
	move_near_obj_combat.AddState(AlwaysFail) // Index 5
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);
	move_near_obj_combat.AddState(GoalPlaySoundScratch5) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(7));
	move_near_obj_combat.AddState(GoalAttackerHasRangedWeapon) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_attempt_move_near_combat))
		.OnFailure(T_PUSH_GOAL(ag_attempt_move_near));

	// ag_attempt_move_near_combat
	auto attempt_move_near_combat = AnimGoalBuilder(goals_[ag_attempt_move_near_combat])
		.SetPriority(AGP_2);
	attempt_move_near_combat.AddCleanup(GoalAttemptMoveCleanup)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_move_near_combat.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_GOTO_STATE(1));
	attempt_move_near_combat.AddState(AlwaysFail) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(9), DELAY_RANDOM)
		.OnFailure(T_GOTO_STATE(2));
	attempt_move_near_combat.AddState(GoalMoveAlongPath) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_GOAL, DELAY_SLOT);
	attempt_move_near_combat.AddState(GoalIsCurrentPathValid) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(11));
	attempt_move_near_combat.AddState(GoalMoveNearUpdateRadiusToReach) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | 0x8000000)
		.OnFailure(T_GOTO_STATE(5));
	attempt_move_near_combat.AddState(AlwaysFail) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(10))
		.OnFailure(T_GOTO_STATE(6));
	attempt_move_near_combat.AddState(GoalHasDoorInPath) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(8));
	attempt_move_near_combat.AddState(GoalIsDoorFullyClosed) // Index 7
		.SetArgs(AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_INVALIDATE_PATH | T_PUSH_GOAL(ag_open_door) | T_REWIND, 50)
		.OnFailure(T_GOTO_STATE(8));
	attempt_move_near_combat.AddState(GoalPlayMoveAnim) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(9));
	attempt_move_near_combat.AddState(GoalActionPerform3) // Index 9
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	attempt_move_near_combat.AddState(GoalIsDoorUnlocked) // Index 10
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SCRATCH_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_jump_window) | T_REWIND)
		.OnFailure(T_POP_ALL);
	attempt_move_near_combat.AddState(AlwaysFail) // Index 11
		.OnSuccess(T_PUSH_GOAL(ag_move_to_pause) | T_REWIND, 1000)
		.OnFailure(T_POP_ALL);

	// ag_use_container
	auto use_container = AnimGoalBuilder(goals_[ag_use_container])
		.SetPriority(AGP_3);
	use_container.AddState(GoalSetRadiusTo2) // Index 0
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_POP_ALL);
	use_container.AddState(GoalIsTargetWithinRadius) // Index 1
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_PUSH_GOAL(ag_move_near_obj));
	use_container.AddState(GoalIsRotatedTowardTarget) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_PUSH_GOAL(ag_rotate));
	use_container.AddState(GoalIsParam1Door) // Index 3
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_GOTO_STATE(4));
	use_container.AddState(GoalUseObject) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);
	use_container.AddState(GoalSaveParam1InScratch) // Index 5
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	use_container.AddState(GoalIsDoorFullyClosed) // Index 6
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_open_door))
		.OnFailure(T_POP_GOAL | T_PUSH_GOAL(ag_close_door));

	// ag_throw_spell_w_cast_anim
	auto throw_spell_w_cast_anim = AnimGoalBuilder(goals_[ag_throw_spell_w_cast_anim])
		.SetPriority(AGP_3)
		.SetRelatedGoals(ag_attempt_spell);
	throw_spell_w_cast_anim.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	throw_spell_w_cast_anim.AddState(AlwaysSucceed) // Index 0
		.SetArgs(AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell_w_cast_anim.AddState(GoalIsProne) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_GOTO_STATE(2));
	throw_spell_w_cast_anim.AddState(GoalIsConcealed) // Index 2
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_unconceal))
		.OnFailure(T_GOTO_STATE(3));
	throw_spell_w_cast_anim.AddState(GoalReturnTrue) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_GOTO_STATE(7));
	throw_spell_w_cast_anim.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(5))
		.OnFailure(T_PUSH_GOAL(ag_pick_weapon));
	throw_spell_w_cast_anim.AddState(AlwaysSucceed) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(8))
		.OnFailure(T_GOTO_STATE(6));
	throw_spell_w_cast_anim.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 6
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_PUSH_GOAL(ag_move_near_obj))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell_w_cast_anim.AddState(AlwaysSucceed) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(9))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell_w_cast_anim.AddState(GoalSetRotationToFaceTargetLoc) // Index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_TILE)
		.OnSuccess(T_POP_GOAL | T_PUSH_GOAL(ag_attempt_spell_w_cast_anim))
		.OnFailure(T_GOTO_STATE(9));
	throw_spell_w_cast_anim.AddState(GoalCastConjureEnd) // Index 9
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_attempt_spell_w_cast_anim
	//
	// Spell casting animation state machine.
	//
	// There are two animations for each spell, "conjuring" and "casting".
	//
	// 1. Conjuring is a loopable animation of the creature's arms swinging.
	// 2. Casting is the animation to play when the spell is about to happen.
	//
	// The state machine makes use of 3 animation flags.
	//
	//   1. 0x4 is set when an 'action' should happen during the animation.
	//   2. 0x8 is set when we're ready to transition from the conjuring to
	//      the casting animation.
	//   3. 0x10 is set when an animation is in progress.
	//
	// Note that 0x4 can be set _before_ 0x10 is cleared (the latter indicating
	// that the animation has ended).
	//
	// For conjuring, the 'action' seems to be a point in the middle of the
	// animation at which transition to casting should take place. The original
	// machine made the mistake of restarting the conjuration animation at this
	// point when it was looping (which happens during interruption).
	//
	// For casting, the 'action' seems to be when the spell 'attempt' should be
	// activated, which triggers the actual effects of the spell. In general
	// there will still be frames left of the casting animation after this
	// point.
	//
	// The logic of the machine is roughly as follows:
	//
	//   - play the conjuring animation through at least once, starting with
	//     the 'trigger' action; the trigger should happen only once, though
	//   - once the conjuration animation has completed, set flag 0x8 and
	//     restart the animation
	//   - when the conjuring action happens, check if the spell has been
	//     interrupted
	//     - if so, just unset 0x4 and keep animating
	//     - if not, and 0x8 is set, switch to the casting animation
	//   - when the casting animation action happens, 'attempt' the spell
	auto attempt_spell_w_cast_anim = AnimGoalBuilder(goals_[ag_attempt_spell_w_cast_anim])
		.SetPriority(AGP_4)
		.SetRelatedGoals(ag_throw_spell);
	attempt_spell_w_cast_anim.AddCleanup(GoalCastConjureEnd)
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.SetFlagsData(1);
	// test if we're animating something
	attempt_spell_w_cast_anim.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(1)) // not animating yet
		.OnFailure(T_GOTO_STATE(3)); // animating
	// see if we've already animated conjuration
	attempt_spell_w_cast_anim.AddState(GoalTestCompletedOnce) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(7)) // we have
		.OnFailure(T_GOTO_STATE(2)); // newly arrived
	// first animation, trigger spell
	attempt_spell_w_cast_anim.AddState(GoalTriggerSpell) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_REWIND)    // start animating
		.OnFailure(T_POP_GOAL); // something went wrong
	// advance one frame
	attempt_spell_w_cast_anim.AddState(GoalUnconcealAnimate) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(4))  // animation continuing
		.OnFailure(T_GOTO_STATE(10)); // animation ended or failed somehow
	// test for animation action
	attempt_spell_w_cast_anim.AddState(GoalTestAction) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(5)) // action indicated
		.OnFailure(T_REWIND, DELAY_SLOT);
	// test for interruption on action
	attempt_spell_w_cast_anim.AddState(GoalWasInterrupted) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(6)) // just clear flag 0x4
		.OnFailure(T_GOTO_STATE(8));
	// unset action flag
	attempt_spell_w_cast_anim.AddState(GoalUnsetAction) // Index 6
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_REWIND, DELAY_SLOT);
	// begin a spell animation without trigger
	// casting animation will be started if 0x8 and 0x4
	attempt_spell_w_cast_anim.AddState(GoalBeginSpellAnim) // index 7
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_POP_GOAL);
	// animation action, check which animation is playing
	attempt_spell_w_cast_anim.AddState(GoalTestAnimatingCasting) // index 8
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(9)) // attempt spell
		.OnFailure(T_GOTO_STATE(7)); // start casting animation
	attempt_spell_w_cast_anim.AddState(GoalAttemptSpell) // index 9
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(6)) // clear action flag
		.OnFailure(T_POP_ALL);
	// animation stopped, check if proper end or error
	attempt_spell_w_cast_anim.AddState(GoalTestAnimating) // index 10
		.OnSuccess(T_POP_GOAL)  // bail-out
		.OnFailure(T_GOTO_STATE(11)); // proper end
	// mark conjuration animation as done once
	attempt_spell_w_cast_anim.AddState(GoalSpellAnimCompleted) // index 11
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_GOTO_STATE(7)) // conjuration, restart animation
		.OnFailure(T_POP_GOAL); // casting, goal finished

	// ag_throw_spell_w_cast_anim_2ndary
	auto throw_spell_w_cast_anim_2ndary = AnimGoalBuilder(goals_[ag_throw_spell_w_cast_anim_2ndary])
		.SetPriority(AGP_5)
		.SetInterruptAll(true)
		.SetFieldC(true);
	throw_spell_w_cast_anim_2ndary.AddCleanup(AlwaysSucceed)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	throw_spell_w_cast_anim_2ndary.AddState(GoalIsNotStackFlagsData40) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	throw_spell_w_cast_anim_2ndary.AddState(GoalSetSlotFlags4) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(4));
	throw_spell_w_cast_anim_2ndary.AddState(AlwaysSucceed) // Index 2
		.SetArgs(AGDATA_SKILL_DATA)
		.OnSuccess(T_GOTO_STATE(3), DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	throw_spell_w_cast_anim_2ndary.AddState(AlwaysSucceed) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	throw_spell_w_cast_anim_2ndary.AddState(AlwaysSucceed) // Index 4
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_GOTO_STATE(2));

	// ag_back_off_from
	auto back_off_from = AnimGoalBuilder(goals_[ag_back_off_from])
		.SetPriority(AGP_3);
	back_off_from.AddState(GoalSetRunningFlag) // Index 0
		.OnSuccess(T_GOTO_STATE(1))
		.OnFailure(T_GOTO_STATE(1));
	back_off_from.AddState(GoalSaveStateDataOrSpellRangeInRadius) // Index 1
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_POP_ALL);
	back_off_from.AddState(GoalIsTargetWithinRadius) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_PUSH_GOAL(ag_move_away_from_obj))
		.OnFailure(T_GOTO_STATE(3));
	back_off_from.AddState(GoalActionPerform3) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_GOAL, DELAY_SLOT)
		.OnFailure(T_POP_ALL);

	// ag_attempt_use_pickpocket_skill_on
	auto attempt_use_pickpocket_skill_on = AnimGoalBuilder(goals_[ag_attempt_use_pickpocket_skill_on])
		.SetPriority(AGP_3);
	attempt_use_pickpocket_skill_on.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	attempt_use_pickpocket_skill_on.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	attempt_use_pickpocket_skill_on.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(7), DELAY_SLOT);
	attempt_use_pickpocket_skill_on.AddState(GoalSetRotationToFaceTargetObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	attempt_use_pickpocket_skill_on.AddState(GoalPickpocketPerform) // Index 3
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_ALL);
	attempt_use_pickpocket_skill_on.AddState(GoalCheckSlotFlag40000) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(7))
		.OnFailure(T_GOTO_STATE(5));
	attempt_use_pickpocket_skill_on.AddState(GoalPlayAnim) // Index 5
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_GOTO_STATE(6))
		.OnFailure(T_POP_ALL);
	attempt_use_pickpocket_skill_on.AddState(GoalThrowItemPlayAnim) // Index 6
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND)
		.OnFailure(T_GOTO_STATE(8));
	attempt_use_pickpocket_skill_on.AddState(GoalPlayAnim) // Index 7
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);
	attempt_use_pickpocket_skill_on.AddState(GoalActionPerform3) // Index 8
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1)
		.OnSuccess(T_POP_ALL)
		.OnFailure(T_POP_ALL);

	// ag_use_disable_device_skill_on_data
	auto use_disable_device_skill_on_data = AnimGoalBuilder(goals_[ag_use_disable_device_skill_on_data])
		.SetPriority(AGP_3);
	use_disable_device_skill_on_data.AddCleanup(GoalResetToIdleAnim)
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(1);
	use_disable_device_skill_on_data.AddState(GoalIsSlotFlag10NotSet) // Index 0
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_GOTO_STATE(2))
		.OnFailure(T_GOTO_STATE(1));
	use_disable_device_skill_on_data.AddState(GoalUnconcealAnimate) // Index 1
		.SetArgs(AGDATA_SELF_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(5), DELAY_SLOT);
	use_disable_device_skill_on_data.AddState(GoalSetRotationToFaceTargetObj) // Index 2
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_GOTO_STATE(3))
		.OnFailure(T_POP_ALL);
	use_disable_device_skill_on_data.AddState(GoalPlayAnim) // Index 3
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(104)
		.OnSuccess(T_GOTO_STATE(4))
		.OnFailure(T_POP_ALL);
	use_disable_device_skill_on_data.AddState(GoalThrowItemPlayAnim) // Index 4
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_ANIM_ID_PREV)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_GOTO_STATE(6));
	use_disable_device_skill_on_data.AddState(GoalAttemptTrapDisarm) // Index 5
		.SetArgs(AGDATA_SELF_OBJ, AGDATA_TARGET_OBJ)
		.OnSuccess(T_REWIND, DELAY_SLOT)
		.OnFailure(T_POP_ALL);
	use_disable_device_skill_on_data.AddState(GoalPlayAnim) // Index 6
		.SetArgs(AGDATA_SELF_OBJ)
		.SetFlagsData(9)
		.OnSuccess(T_POP_GOAL)
		.OnFailure(T_POP_ALL);


}

const AnimGoal & AnimationGoals::GetByType(AnimGoalType type) const
{
	using AnimGoalArray = const AnimGoal*[82];
	static auto mGoals = temple::GetRef<AnimGoalArray>(0x102BD1B0);

	//return *mGoals[type];

	auto it = goals_.find(type);
	if (it != goals_.end()) {
		return it->second;
	}
	throw TempleException("Unknown anim goal type {}", type);
}

bool AnimationGoals::IsValidType(AnimGoalType type) const
{
	return type >= 0 && type < ag_count;
}


#pragma once

#include <cstdint>

struct AnimSlot;

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

enum AnimStateTransitionFlags : uint32_t
{
	ASTF_GOAL_DESTINATION_REMOVE = 0x2000000,
	ASTF_REWIND = 0x10000000, // will transition back to state 0
	ASTF_POP_GOAL = 0x30000000,
	ASTF_POP_GOAL_TWICE = 0x38000000,
	ASTF_PUSH_GOAL = 0x40000000,
	ASTF_POP_ALL = 0x90000000,
	ASTF_MASK = 0xFF000000 // the general mask for the special state transition flags
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
	int interruptAll;
	int field_C;
	int field_10;
	int relatedGoal1;
	int relatedGoal2;
	int relatedGoal3;
	AnimGoalState states[16];
	AnimGoalState state_special;
};

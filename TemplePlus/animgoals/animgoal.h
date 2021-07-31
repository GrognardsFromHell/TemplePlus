
#pragma once

#include <cstdint>
#include <fmt/format.h>

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

enum AnimGoalProperty {
	AGDATA_SELF_OBJ = 0,     // Type: 1 (Object)
	AGDATA_TARGET_OBJ,       // Type: 1
	AGDATA_BLOCK_OBJ,        // Type: 1
	AGDATA_SCRATCH_OBJ,      // Type: 1
	AGDATA_PARENT_OBJ,       // Type: 1
	AGDATA_TARGET_TILE,      // Type: 2 (Location)
	AGDATA_RANGE_DATA,       // Type: 2
	AGDATA_ANIM_ID,          // Type: 0 (just a 32-bit number it seems)
	AGDATA_ANIM_ID_PREV, // Type: 0
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
	AGDATA_SOUND_HANDLE,      // Type: 0

	SELF_OBJ_PRECISE_LOC = 31,
	TARGET_OBJ_PRECISE_LOC = 32,
	NULL_HANDLE = 33,
	TARGET_LOC_PRECISE = 34
};

std::string_view GetAnimGoalPriorityText(AnimGoalPriority priority);

namespace fmt {
    template<>
    struct formatter<AnimGoalPriority> : simple_formatter {
        template<typename FormatContext>
        auto format(const AnimGoalPriority &priority, FormatContext &ctx) {
            return format_to(ctx.out(), GetAnimGoalPriorityText(priority));
        }
    };
}

enum AnimStateTransitionFlags : uint32_t
{
	ASTF_GOAL_INVALIDATE_PATH = 0x2000000,
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
	int flagsData;
	AnimStateTransition afterFailure;
	AnimStateTransition afterSuccess;
};

struct AnimGoal {
	int statecount = 0;
	AnimGoalPriority priority = AGP_NONE;
	int interruptAll = false;
	int field_C = 0; // Indicates that it should be saved
	int field_10 = 0;
	int relatedGoal[3] = { 0, 0, 0 };
	AnimGoalState states[16];
	AnimGoalState state_special = { 0, };
};

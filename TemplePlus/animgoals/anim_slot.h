
#pragma once

#include "obj.h"
#include "animgoals_stackentry.h"
#include "pathfinding.h"
#include "gametime.h"

#pragma pack(push, 1)

struct LineRasterPacket {
	int counter;
	int interval;
	int deltaIdx;
	int unused;
	int64_t x;
	int64_t y;
	int8_t * deltaXY;

	LineRasterPacket() {
		counter = 0;
		interval = 10;
		deltaIdx = 0;
		x = 0i64;
		y = 0i64;
		deltaXY = nullptr;

	}
};

enum AnimSlotFlag {
	ASF_ACTIVE = 1,
	ASF_STOP_PROCESSING = 2, // Used in context with "killing the animation slot"
	ASF_UNK3 = 4, // Seen in goalstatefunc_82, goalstatefunc_83, set with 0x8 in
	// goalstatefunc_42
	ASF_ACTION = 4,
	ASF_UNK4 = 8, // Seen in goalstatefunc_82, goalstatefunc_83, set with 0x8 in
	// goalstatefunc_42
	ASF_UNK5 = 0x10, // Seen in goalstatefunc_84_animated_forever, set in
	// goalstatefunc_87
	ASF_ANIMATING = 0x10,
	ASF_UNK7 = 0x20, // Seen as 0x30 is cleared in goalstatefunc_7 and goalstatefunc_8
	ASF_RUNNING = 0x40,
	ASF_SPEED_RECALC = 0x80,
	ASF_UNK8 = 0x400,   // Seen in goal_calc_path_to_loc, goalstatefunc_13_rotate,
	// set in goalstatefunc_18
	ASF_UNK10 = 0x800,  // Seen in goalstatefunc_37
	ASF_UNK9 = 0x4000,  // Seen in goalstatefunc_19
	ASF_UNK11 = 0x8000, // Test in goalstatefunc_43
	ASF_UNK1 = 0x10000,
	ASF_UNK6 = 0x20000, // Probably sound related (seen in anim_goal_free_sound_id)
	ASF_UNK12 = 0x40000 // set goalstatefunc_48, checked in goalstatefunc_50
};

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
	AnimPath animPath;
	PathQueryResult path;
	AnimParam param1; // Used as parameters for goal states
	AnimParam param2; // Used as parameters for goal states
	uint32_t stateFlagData;
	uint32_t unknown[5];
	GameTime gametimeSth;
	uint32_t currentPing;    // I.e. used in
	uint32_t uniqueActionId; // ID assigned when triggered by a D20 action

	bool IsActive() const {
		return (flags & ASF_ACTIVE) != 0;
	}

	bool IsStopProcessing() const {
		return (flags & ASF_STOP_PROCESSING) != 0;
	}

	bool IsStackFull() const {
		return currentGoal >= 7;
	}

	bool IsStackEmpty() const {
		return currentGoal < 0;
	}

	void Clear() {
		id.Clear();
		pCurrentGoal = nullptr;
		animObj = objHndl::null;
		flags = 0;
		currentGoal = -1;
		animPath.flags = 0;
	}

};

#pragma pack(pop)


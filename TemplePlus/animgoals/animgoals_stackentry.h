
#pragma once

#include "obj.h"
#include "anim.h"
#include "animgoal.h"
#include "frozen_obj_ref.h"

// Has to be 0x10 in size
union AnimParam {
	objHndl obj;
	LocAndOffsets location;
	int number;
	int spellId;
	float floatNum;
};

struct AnimSlotGoalStackEntry {
	AnimGoalType goalType;
	int unk1;
	AnimParam self; // 0x8
	AnimParam target; // 0x18
	AnimParam block; // 0x28
	AnimParam scratch; //0x38
	AnimParam parent; // 0x48
	AnimParam targetTile; //0x58
	AnimParam range; //0x68
	AnimParam animId; //0x78
	AnimParam animIdPrevious; // 0x88
	AnimParam animData; // 0x98
	AnimParam spellData; // 0xA8
	AnimParam skillData; // 0xB8
	AnimParam flagsData; // 0xC8
	AnimParam scratchVal1; // 0xD8
	AnimParam scratchVal2; //0xE8
	AnimParam scratchVal3;
	AnimParam scratchVal4;
	AnimParam scratchVal5;
	AnimParam scratchVal6;
	AnimParam soundHandle;
	uint32_t soundStreamId;
	uint32_t soundStreamId2; // Probably padding
	uint32_t padding[2];
	FrozenObjRef selfTracking;
	FrozenObjRef targetTracking;
	FrozenObjRef blockTracking;
	FrozenObjRef scratchTracking;
	FrozenObjRef parentTracking;

	bool InitWithInterrupt(objHndl obj, AnimGoalType goalType);
	bool Push(AnimSlotId* idNew);
	bool Init(objHndl handle, AnimGoalType, bool withInterrupt = 0);
	void FreezeObjectRefs();
	bool ValidateObjectRefs();

	AnimSlotGoalStackEntry(objHndl handle, AnimGoalType, bool withInterrupt = false);
	AnimSlotGoalStackEntry() { memset(this, 0, sizeof(AnimSlotGoalStackEntry)); };
};

const auto TestSizeOfAnimSlotGoalStackEntry = sizeof(AnimSlotGoalStackEntry); // should be 544 (0x220)



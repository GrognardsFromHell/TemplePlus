
#include "stdafx.h"

#include <temple/dll.h>

#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/mapsystem.h"
#include "animgoals_stackentry.h"

static_assert(temple::validate_size<AnimSlotGoalStackEntry, 0x220>::value);
static_assert(temple::validate_size<AnimParam, 0x10>::value);

// Originally @ 0x100556C0
bool AnimSlotGoalStackEntry::InitWithInterrupt(objHndl handle, AnimGoalType goalType) {
	return Init(handle, goalType, true);
}

bool AnimSlotGoalStackEntry::Push(AnimSlotId* idNew) {
	return gameSystems->GetAnim().PushGoal(*this, idNew);
}

// Originally @ 0x10055570
bool AnimSlotGoalStackEntry::Init(objHndl handle, AnimGoalType goalType, bool withInterrupt) {
	auto gdata = this;

	if (!gdata) {
		logger->error("Null goalData ptr");
		gameSystems->GetAnim().Debug();
	}

	if ((goalType & 0x80000000) || goalType >= ag_count) {
		logger->error("Illegal goalType");
		gameSystems->GetAnim().Debug();
	}
	gdata->animId.number = -1;
	gdata->animIdPrevious.number = -1;
	gdata->animData.number = -1;
	gdata->spellData.number = -1;
	gdata->flagsData.number = -1;
	gdata->soundStreamId = -1;
	gdata->goalType = goalType;
	gdata->self.obj = handle;
	gdata->target.obj = objHndl::null;
	gdata->block.obj = objHndl::null;
	gdata->scratch.obj = objHndl::null;
	gdata->parent.obj = objHndl::null;
	gdata->targetTile.location = LocAndOffsets::null;
	gdata->range.location = LocAndOffsets::null;
	gdata->skillData.number = 0;
	gdata->scratchVal1.number = 0;
	gdata->scratchVal2.number = 0;
	gdata->scratchVal3.number = 0;
	gdata->scratchVal4.number = 0;
	gdata->scratchVal5.number = 0;
	gdata->scratchVal6.number = 0;
	gdata->soundHandle.number = 0;
	if (withInterrupt)
	{
		auto ag = gameSystems->GetAnim().GetGoal(goalType);
		if (!ag) {
			logger->error("pGoalNode != NULL assertion failed");
			gameSystems->GetAnim().Debug();
		}
		return gameSystems->GetAnim().Interrupt(handle, ag->priority, ag->interruptAll);
	}
	return TRUE;
}

void AnimSlotGoalStackEntry::FreezeObjectRefs()
{
	selfTracking = FrozenObjRef::Freeze(self.obj);
	targetTracking = FrozenObjRef::Freeze(target.obj);
	blockTracking = FrozenObjRef::Freeze(block.obj);
	scratchTracking = FrozenObjRef::Freeze(scratch.obj);
	parentTracking = FrozenObjRef::Freeze(parent.obj);
}

static bool ValidateObjectRef(objHndl &handle, const FrozenObjRef &frozenRef) {
	if (!objSystem->IsValidHandle(handle)) {
		if (FrozenObjRef::Unfreeze(frozenRef, &handle)) {
			logger->error("Failed to recover object reference in anim goal");
			handle = objHndl::null;
			return false;
		}
	}
	return true;
}

bool AnimSlotGoalStackEntry::ValidateObjectRefs()
{
	if (gameSystems->GetMap().IsClearingMap()) {
		self.obj = objHndl::null;
		target.obj = objHndl::null;
		block.obj = objHndl::null;
		scratch.obj = objHndl::null;
		parent.obj = objHndl::null;
		return false;
	}

	if (!ValidateObjectRef(self.obj, selfTracking)) {
		return false;
	}
	if (!ValidateObjectRef(target.obj, targetTracking)) {
		return false;
	}
	if (!ValidateObjectRef(block.obj, blockTracking)) {
		return false;
	}
	if (!ValidateObjectRef(scratch.obj, scratchTracking)) {
		return false;
	}
	if (!ValidateObjectRef(parent.obj, parentTracking)) {
		return false;
	}

	return true;
}

AnimSlotGoalStackEntry::AnimSlotGoalStackEntry(objHndl handle, AnimGoalType GoalType, bool withInterrupt) {
	Init(handle, GoalType, withInterrupt);
}

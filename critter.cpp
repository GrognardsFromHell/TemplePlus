#include "stdafx.h"
#include "common.h"
#include "critter.h"
#include "obj.h"


class CritterReplacements : public TempleFix
{
	macTempleFix(Critter System)
	{
		logger->info("Replacing Critter System functions");
		macReplaceFun(10062720, _isCritterCombatModeActive)
	}
} critterReplacements;


#pragma region Critter System Implementation

struct CritterSystem critterSys;

uint32_t CritterSystem::isCritterCombatModeActive(objHndl objHnd)
{
	return (objects.getInt32(objHnd, obj_f_critter_flags) & OCF_COMBAT_MODE_ACTIVE) != 0;
}

CritterSystem::CritterSystem()
{
	
}
#pragma endregion


#pragma region Critter Hooks

uint32_t _isCritterCombatModeActive(objHndl objHnd)
{
	return critterSys.isCritterCombatModeActive(objHnd);
}
#pragma endregion 
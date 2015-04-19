#pragma once
#include "stdafx.h"
#include "common.h"


struct CritterSystem : AddressTable
{

	uint32_t isCritterCombatModeActive(objHndl objHnd);
	CritterSystem();
};

extern CritterSystem critterSys;

uint32_t _isCritterCombatModeActive(objHndl objHnd);
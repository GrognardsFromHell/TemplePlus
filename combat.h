#pragma once

#include "stdafx.h"
#include "util/addresses.h"
#include "tig/tig_mes.h"


uint32_t IsCombatActive();
uint32_t Combat_GetMesfileIdx_CombatMes();

struct CombatSystem : AddressTable
{
	MesHandle * combatMesfileIdx;
	uint32_t * combatModeActive;

	CombatSystem()
	{
		rebase(combatModeActive, 0x10AA8418);
		rebase(combatMesfileIdx, 0x10AA8408);
	}
} ;

extern CombatSystem combat;
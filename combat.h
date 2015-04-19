#pragma once

#include "stdafx.h"
#include "util/addresses.h"
#include "tig/tig_mes.h"


uint32_t Combat_GetMesfileIdx_CombatMes();

struct CombatSystem : AddressTable
{
	MesHandle * combatMesfileIdx;
	uint32_t * combatModeActive;
	bool isCombatActive();

	/*
		Use for the non-lethal brawl.
	*/
	void (__cdecl *Brawl)(objHndl a, objHndl b);

	CombatSystem()
	{
		rebase(combatModeActive, 0x10AA8418);
		rebase(combatMesfileIdx, 0x10AA8408);
		rebase(Brawl, 0x100EBD40);
	}
} ;

extern CombatSystem combatSys;



uint32_t _isCombatActive();
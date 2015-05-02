#pragma once

#include "stdafx.h"
#include "util/addresses.h"
#include "tig/tig_mes.h"


uint32_t Combat_GetMesfileIdx_CombatMes();

struct CombatSystem : AddressTable {
	MesHandle* combatMesfileIdx;
	uint32_t* combatModeActive;
	bool isCombatActive();

	/*
		Use for the non-lethal brawl.
	*/
	void (__cdecl *Brawl)(objHndl a, objHndl b);
	void enterCombat(objHndl objHnd);

	void (__cdecl *AddToInitiative)(objHndl critter);
	void (__cdecl *RemoveFromInitiative)(objHndl critter);

	int (__cdecl *GetInitiative)(objHndl critter);
	void (__cdecl *SetInitiative)(objHndl critter, int initiative);

	CombatSystem() {
		rebase(combatModeActive, 0x10AA8418);
		rebase(combatMesfileIdx, 0x10AA8408);
		macRebase(_enterCombat, 100631E0)
		rebase(Brawl, 0x100EBD40);
		rebase(AddToInitiative, 0x100B2D50);
		rebase(RemoveFromInitiative, 0x100B2D80);
		rebase(GetInitiative, 0x100DEDB0);
		rebase(SetInitiative, 0x100DF2E0);

	}

private:
	void (__cdecl *_enterCombat)(objHndl objHnd);

};

extern CombatSystem combatSys;


uint32_t _isCombatActive();

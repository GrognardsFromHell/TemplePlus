#pragma once
#include "common.h"
#include <temple/dll.h>

struct FactionSystem : temple::AddressTable
{
	uint32_t(__cdecl *FactionHas)(objHndl, uint32_t nFaction);
	uint32_t(__cdecl *PCHasFactionFromReputation)(objHndl, uint32_t nFaction);
	uint32_t(__cdecl *FactionAdd)(objHndl, uint32_t nFaction);

	FactionSystem()
	{
		rebase(FactionHas, 0x1007E430);
		rebase(FactionAdd, 0x1007E480);
		rebase(PCHasFactionFromReputation, 0x10054D70);
	}
};

extern FactionSystem factions;
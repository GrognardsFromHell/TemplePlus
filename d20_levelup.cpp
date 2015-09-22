#include "stdafx.h"
#include "d20_levelup.h"
#include "common.h"
#include "obj.h"


struct D20LevelupSystemAddresses : temple::AddressTable
{
	int(__cdecl* LevelupApply)(objHndl objHnd, LevelupPacket* lvlPkt);
	
	void(__cdecl * LevelupStateInit)(LevelupState *lvlStat, objHndl objHnd); // uses critter_pad_i_4 and critter_pad_i_5
	void(__cdecl* ApplyLevelFromLevelupTabEntry)(objHndl objHnd);
	
	D20LevelupSystemAddresses()
	{
		rebase(LevelupApply, 0x100731E0);
		rebase(LevelupStateInit, 0x100F37C0); 
		rebase(ApplyLevelFromLevelupTabEntry, 0x100F3870); 
		
	}
} addresses;
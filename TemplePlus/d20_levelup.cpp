#include "stdafx.h"
#include "d20_levelup.h"
#include "common.h"
#include "obj.h"
#include "util/fixes.h"
#include "config/config.h"
#include "temple_functions.h"


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

static class D20LevelupHooks : public TempleFix
{
public: 
	static int DiceRollHooked(int min, int max, int bonus)
	{
		auto cfgLower(tolower(config.hpOnLevelup));
		
		int result;
		if (!stricmp(cfgLower.c_str(), "max")){
			result = max;
		} else if (!stricmp(cfgLower.c_str(), "average")){
			result = (min + max) / 2  + templeFuncs.RNG(0,1); // hit die are always even numbered so randomize the roundoff
		} else
		{
			result = templeFuncs.RNG(min, max);
		}
		
		return result;
	};

	void apply() override {
		redirectCall(0x100733AC, DiceRollHooked);
	}
} hooks;
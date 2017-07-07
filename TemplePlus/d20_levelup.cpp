#include "stdafx.h"
#include "d20_levelup.h"
#include "common.h"
#include "obj.h"
#include "util/fixes.h"
#include "config/config.h"
#include "temple_functions.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/deity/legacydeitysystem.h"
#include "ui/ui_char_editor.h"
#include "rng.h"


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

	static BOOL IsNonClassSkillHook(SkillEnum skill, Stat classEnum);

	static int DiceRollHooked(int min, int max, int bonus){
		auto cfgLower(tolower(config.hpOnLevelup));
		
		int result;
		if (!_stricmp(cfgLower.c_str(), "max")){
			result = max;
		} else if (!_stricmp(cfgLower.c_str(), "average")){
			result = (min + max) / 2  + rngSys.GetInt(0,1); // hit die are always even numbered so randomize the roundoff
		} else
		{
			result = rngSys.GetInt(min, max);
		}
		
		return result;
	};

	void apply() override {
		redirectCall(0x100733AC, DiceRollHooked);

		redirectCall(0x10180D84, IsNonClassSkillHook); // PcCreationSkillsRender
		redirectCall(0x1018102D, IsNonClassSkillHook); // PcCreationSkillsTextRender
		redirectCall(0x101AB3BA, IsNonClassSkillHook);
		redirectCall(0x101AB68E, IsNonClassSkillHook);
		redirectCall(0x101AB714, IsNonClassSkillHook);
		redirectCall(0x101AB8F1, IsNonClassSkillHook);
		
	}
} hooks;


BOOL D20LevelupHooks::IsNonClassSkillHook(SkillEnum skill, Stat classEnum){
	auto handle = chargen.GetEditedChar();
	if (handle){
		if (d20Sys.D20QueryPython(handle, "Is Class Skill", skill))
			return FALSE;
	}

	return !d20ClassSys.IsClassSkill(skill, classEnum);
}

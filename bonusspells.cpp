
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "temple_functions.h"
#include "obj.h"



void removeSurplusSpells(int surplus, objHndl objHnd, uint32_t classCode, int slotLvl)
{
	int numMemorized = templeFuncs.Obj_Get_IdxField_NumItems(objHnd, obj_f_critter_spells_memorized_idx);
	SpellStoreData spellData;
	for (int i = numMemorized - 1; i>=0 && surplus > 0 ; i--)
	{
		templeFuncs.Obj_Get_ArrayElem_Generic(objHnd, obj_f_critter_spells_memorized_idx, i, &spellData);
		if (spellData.classCode == (classCode | 0x80) && slotLvl == spellData.spellLevel)
		{
			spellSys.spellRemoveFromStorage(objHnd, obj_f_critter_spells_memorized_idx, &spellData, 0);
			surplus--;
		}
	}

}

int getMemorizedSpells(objHndl objHnd, uint32_t classCode, int slotLvl)
{
	int numMemorizedThisLvl = 0;
	int memorizedTotal = templeFuncs.Obj_Get_IdxField_NumItems(objHnd, obj_f_critter_spells_memorized_idx);
	SpellStoreData spellData ;
	for (int i = 0; i < memorizedTotal; i++)
	{
		templeFuncs.Obj_Get_ArrayElem_Generic(objHnd, obj_f_critter_spells_memorized_idx, i, &spellData);
		if (spellData.classCode == (classCode | 0x80 ) && spellData.spellLevel == slotLvl)
		{
			numMemorizedThisLvl++;
		}
	}
	return numMemorizedThisLvl;
}

int getMaxSpells(objHndl objHnd, uint32_t classCode, int slotLvl, uint32_t classLvl)
{
	uint32_t maxSpells = spellSys.getBaseSpellCountByClassLvl(classCode, classLvl, slotLvl, 0);
	if (maxSpells)
	{
		if ( spellSys.getWizSchool(objHnd) )
		{
			maxSpells++;

		}
		maxSpells += spellSys.getStatModBonusSpellCount(objHnd, classCode, slotLvl);
		return maxSpells;
	} 
	else if ( d20ClassSys.IsLateCastingClass((Stat)classCode) 
		      && (classLvl >= 4 && slotLvl == 1 
				  || classLvl >= 8 && slotLvl == 2
				  || classLvl >= 11 && slotLvl == 3
				  || classLvl >= 14 && slotLvl == 4))
	{
		maxSpells += spellSys.getStatModBonusSpellCount(objHnd, classCode, slotLvl);
			return maxSpells;
	}

	return 0;
	
}

void doSanitize(objHndl objHnd, uint32_t classCode, uint32_t classLvl)
{
	uint32_t maxSpells = 0;
	uint32_t numSpells = 0;
	for (auto slotLvl = 1; slotLvl < 10; slotLvl ++)
	{
		maxSpells = getMaxSpells(objHnd, classCode, slotLvl, classLvl);
		numSpells = getMemorizedSpells(objHnd, classCode, slotLvl);
		if (numSpells > maxSpells)
		{
			removeSurplusSpells(numSpells - maxSpells, objHnd, classCode, slotLvl);
		}
	};

}


static void __cdecl sanitizeSpellSlots(objHndl objHnd) {
	auto clrLvl = objects.StatLevelGet(objHnd, stat_level_cleric);
	if (clrLvl)
	{
		doSanitize(objHnd, stat_level_cleric, clrLvl);
	}
	auto drdLvl = objects.StatLevelGet(objHnd, stat_level_druid);
	if (drdLvl)
	{
		doSanitize(objHnd, stat_level_druid, drdLvl);
	}
	auto pldLvl = objects.StatLevelGet(objHnd, stat_level_paladin);
	if (pldLvl)
	{
		doSanitize(objHnd, stat_level_paladin, pldLvl);
	}
	auto rgrLvl = objects.StatLevelGet(objHnd, stat_level_ranger);
	if (rgrLvl)
	{
		doSanitize(objHnd, stat_level_ranger, rgrLvl);
	}
	auto wizLvl = objects.StatLevelGet(objHnd, stat_level_wizard);
	if (wizLvl)
	{
		doSanitize(objHnd, stat_level_wizard, wizLvl);
	}
}


void __cdecl hookedSpellsPendingToMemorized(objHndl objHnd)
{
	sanitizeSpellSlots(objHnd);
	spellSys.spellsPendingToMemorized(objHnd);
}

class BonusSpellFix : public TempleFix {
public:
	const char* name() override {
		return "bonus spell fix";
	}
	
	void apply() override {
		redirectCall(0x100F4C65, _abilityScoreLevelGet);
		redirectCall(0x1010EFE8, hookedSpellsPendingToMemorized);
	}
} bonusSpellFix;


#include "stdafx.h"
#include "spell_structs.h"
#include "spell.h"
#include "d20.h"


void D20SpellData::Extract(int* SpellEnum, int *SpellEnumOrg, int* SpellClass, int* SpellLevel, int* InvIdx, MetaMagicData* MmData) {

	if (SpellEnumOrg){
		*SpellEnumOrg = spellEnumOrg;
	}

	if (SpellLevel){
		*SpellLevel= spellSlotLevel;
	}

	if (SpellEnum){
		if (spontCastType == spontCastGoodCleric){
			*SpellEnum = spontCastSpellLists.spontCastSpellsGoodCleric[spellSlotLevel];
		}
		else if (spontCastType == spontCastEvilCleric){
			*SpellEnum = spontCastSpellLists.spontCastSpellsEvilCleric[spellSlotLevel];
		}
		else if (spontCastType == spontCastDruid){
			*SpellEnum = spontCastSpellLists.spontCastSpellsDruid[spellSlotLevel];
		}
		else{
			*SpellEnum = spellEnumOrg;
		}
	}

	if (SpellClass){
		*SpellClass = spellClassCode;
	}

	if (InvIdx){
		*InvIdx  = itemSpellData;
	}

	if (MmData){
		*MmData = metaMagicData;
	}
}

SpellStoreData::SpellStoreData(int SpellEnum, int SpellLevel, int ClassCode, int mmData, int SpellStoreData):SpellStoreData() {
	spellEnum = SpellEnum;
	classCode = ClassCode;
	spellLevel = SpellLevel;
	metaMagicData = MetaMagicData(mmData);
	spellStoreState = SpellStoreState(SpellStoreData);
	padSpellStore = 0;	
}

bool SpellStoreData::operator<(const SpellStoreData& sp2){
	auto &sp1 = *this;
	int levelDelta = sp1.spellLevel - sp2.spellLevel;
	if (levelDelta < 0)
		return true;
	else if (levelDelta > 0)
		return false;

	// if levels are equal
	auto name1 = spellSys.GetSpellName(sp1.spellEnum);
	auto name2 = spellSys.GetSpellName(sp2.spellEnum);
	auto nameCmp = _strcmpi(name1, name2);
	return nameCmp < 0;
}

SpellComponentFlag SpellStoreData::GetSpellComponentFlags(){

	SpellEntry spEntry(this->spellEnum);

	if (spEntry.spellEnum == 0){
		logger->error("Could not find spell {}", this->spellEnum);
		return (SpellComponentFlag)0;
	}

	auto result = spEntry.spellComponentBitmask;
	if (metaMagicData.metaMagicFlags & MetaMagic_Silent){
		result &= ~SpellComponent_Verbal;
	}
	if (metaMagicData.metaMagicFlags & MetaMagic_Still) {
		result &= ~SpellComponent_Somatic;
	}

	return (SpellComponentFlag)result;
}

bool operator<(const SpellStoreData & sp1, const SpellStoreData & sp2){
	int levelDelta = sp1.spellLevel - sp2.spellLevel;
	if (levelDelta < 0)
		return true;
	else if (levelDelta > 0)
		return false;

	// if levels are equal
	auto name1 = spellSys.GetSpellName(sp1.spellEnum);
	auto name2 = spellSys.GetSpellName(sp2.spellEnum);
	auto nameCmp = _strcmpi(name1, name2);
	return nameCmp < 0;
}

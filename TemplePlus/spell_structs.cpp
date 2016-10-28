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

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

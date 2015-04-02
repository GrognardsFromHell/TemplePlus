#include "stdafx.h"
#include "d20.h"

static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok

extern SpontCastSpellLists spontCastSpellLists_spells;

void D20SpellDataExtractInfo
  (D20SpellData * d20SpellData	, uint32_t * spellEnum		, uint32_t * spellEnumOriginal	, 
   uint32_t * spellClassCode	, uint32_t * spellSlotLevel	, uint32_t * itemSpellData		, 
   uint32_t * metaMagicData)
{
	if ( ! (spellEnumOriginal == nullptr) )
	{
		*spellEnumOriginal = d20SpellData->spellEnumOriginal;
	}

	if (!(spellSlotLevel == nullptr ) )
	{
		*spellSlotLevel = d20SpellData->spellSlotLevel;
	}

	if (! (spellEnum == nullptr))
	{
		if ((SpontCastType) d20SpellData->spontCastType == spontCastGoodCleric)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsGoodCleric[d20SpellData->spellSlotLevel];
		}
		else if ((SpontCastType)d20SpellData->spontCastType == spontCastEvilCleric)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsEvilCleric[d20SpellData->spellSlotLevel];
		} 
		else if ( (SpontCastType) d20SpellData->spontCastType == spontCastDruid)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsDruid[d20SpellData->spellSlotLevel];
		} else
		{
			*spellEnum = d20SpellData->spellEnumOriginal;
		}
	}

	if (!(spellClassCode == nullptr))
	{
		*spellClassCode = d20SpellData->spellClassCode;
	}

	if (!(itemSpellData== nullptr))
	{
		*itemSpellData = d20SpellData->itemSpellData;
	}

	if (!(metaMagicData== nullptr))
	{
		*metaMagicData =  (* ( (uint32_t*) &(d20SpellData->metaMagicData) ) ) & ( 0xFFFFFF); // sue me! it WORKS
	}

	return;
}


void __cdecl D20SpellDataSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType)
{
	if (!(d20SpellData == nullptr))
	{
		d20SpellData->spontCastType = spontCastType;
	}
};


class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
	}
} d20Replacements;
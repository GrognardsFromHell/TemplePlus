#pragma once

#include "temple_functions.h"
#include "spell.h"
#include "feat.h"
#include "fixes.h"
#include "obj.h"

enum SpontCastType : unsigned char {
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

struct D20SpellData
{
	uint16_t spellEnumOriginal;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode ;
	uint8_t itemSpellData;
	SpontCastType spontCastType: 4;
	unsigned char spellSlotLevel : 4;
};

const uint32_t TestSizeOfD20SpellData = sizeof(D20SpellData);


void __cdecl D20SpellDataSetSpontCast(D20SpellData*, SpontCastType spontCastType);
void D20SpellDataExtractInfo
(D20SpellData * d20SpellData, uint32_t * spellEnum, uint32_t * spellEnumOriginal,
uint32_t * spellClassCode, uint32_t * spellSlotLevel, uint32_t * itemSpellData,
uint32_t * metaMagicData);
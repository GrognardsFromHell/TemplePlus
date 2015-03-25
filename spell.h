#pragma once

#include "stdafx.h"
#include "temple_functions.h"

const uint32_t SPELLENUMMAX = 4000;

struct SpellStoredData
{
	uint32_t spellEnum;
	uint32_t classCode;
	uint32_t spellLevel;
	uint32_t spellStoreState; // 1 is for spellsKnown, 2 is for spellsMemorized, 3 is for spellsCast; 2nd byte is for "used up" (relevant for spellsMemorized only)
	uint32_t metaMagicData; 
	/* metaMagicData format: stored Byte-wise
		[0](LSB) - Flags: 1 Maximize Spell count   2  Quicken Spell   4 Silent Spell   8 Still Spell
		[1] - Empower Spell count
		[2] - Enlarge Spell count
		[3] - Extend Spell count
		[4] - Heighten Spell count
		[5] - Widen Spell count
	*/
	uint32_t pad1;
	uint32_t pad2;
	uint32_t pad3;

};
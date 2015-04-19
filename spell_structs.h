#pragma once

#include "stdafx.h"



const uint32_t SPELL_ENUM_MAX = 4000;


enum SpellStoreType : uint8_t
{
	spellStoreKnown = 1,
	spellStoreMemorized = 2,
	spellStoreCast = 3
};

struct MetaMagicData
{
	unsigned char metaMagicFlags : 4; // 1 - Maximize Spell ; 2 - Quicken Spell ; 4 - Silent Spell;  8 - Still Spell
	unsigned char metaMagicEmpowerSpellCount : 4;
	unsigned char metaMagicEnlargeSpellCount : 4;
	unsigned char metaMagicExtendSpellCount : 4;
	unsigned char metaMagicHeightenSpellCount : 4;
	unsigned char metaMagicWidenSpellCount : 4;
};


struct SpellStoreState
{
	SpellStoreType spellStoreType;
	uint8_t usedUp; // relevant only for spellStoreMemorized
};

struct SpellStoreData
{
	uint32_t spellEnum;
	uint32_t classCode;
	uint32_t spellLevel;
	SpellStoreState spellStoreState;
	MetaMagicData metaMagicData; // should be stored as 32bit value!
	char pad0;
	uint32_t pad1;
	uint32_t pad2;
	uint32_t pad3;
};
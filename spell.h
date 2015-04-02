#pragma once

#include "stdafx.h"
#include "temple_functions.h"

const uint32_t SPELLENUMMAX = 4000;
/*

*/
class SpontCastSpellLists
{
public:
	uint32_t spontCastSpellsDruid[11];
	uint32_t spontCastSpellsEvilCleric[11];
	uint32_t spontCastSpellsGoodCleric[11];
	uint32_t spontCastSpellsDruidSummons[11];
	SpontCastSpellLists();


};


//extern SpontCastSpellLists spontCastSpellLists;


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
	SpellStoreState spellStoreState ;
	MetaMagicData metaMagicData; // should be stored as 32bit value!
	char pad0;
	uint32_t pad1;
	uint32_t pad2;
	uint32_t pad3;
};

uint32_t _DruidRadialSelectSummons(uint32_t spellSlotLevel);
void DruidRadialSelectSummonsHook();
uint32_t _DruidRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel);
void DruidRadialSpontCastSpellEnumHook();
uint32_t _GoodClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel);
uint32_t _EvilClericRadialSpontCastSpellEnumHook(uint32_t spellSlotLevel);

const uint32_t TestSizeOfSpellStoreData = sizeof(SpellStoreData);

const uint32_t TestSizeOfMetaMagicData = sizeof(MetaMagicData);
const uint32_t TestSizeOfSpellStoreType = sizeof(SpellStoreType);
const uint32_t TestSizeOfSpellStoreState = sizeof(SpellStoreState);

//const uint32_t bbb = sizeof(int32_t);
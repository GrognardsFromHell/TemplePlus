#pragma once

#include "stdafx.h"
//#include "temple_functions.h"
#include "common.h"


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


struct SpontCastSpellLists : AddressTable
{
public:
	uint32_t spontCastSpellsDruid[11];
	uint32_t spontCastSpellsEvilCleric[11];
	uint32_t spontCastSpellsGoodCleric[11];
	uint32_t spontCastSpellsDruidSummons[11];
	SpontCastSpellLists()
	{
		uint32_t _spontCastSpellsDruid[] = { -1, 476, 477, 478, 479, 480, 481, 482, 483, 484, 4000 };
		uint32_t _spontCastSpellsEvilCleric[] = { 248, 247, 249, 250, 246, 61, 581, 582, 583, 583, 0 };
		uint32_t _spontCastSpellsGoodCleric[] = { 91, 90, 92, 93, 89, 221, 577, 578, 579, 579, 0 };
		uint32_t _spontCastSpellsDruidSummons[] = { -1, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 0 };
		memcpy(spontCastSpellsDruid, _spontCastSpellsDruid, 11 * sizeof(uint32_t));
		memcpy(spontCastSpellsEvilCleric, _spontCastSpellsEvilCleric, 11 * sizeof(uint32_t));
		memcpy(spontCastSpellsGoodCleric, _spontCastSpellsGoodCleric, 11 * sizeof(uint32_t));
		memcpy(spontCastSpellsDruidSummons, _spontCastSpellsDruidSummons, 11 * sizeof(uint32_t));
	}
};

extern SpontCastSpellLists spontCastSpellLists;
#pragma once
#include "stdafx.h"
#include "common.h"


struct D20ClassSystem : AddressTable
{
public:
	Stat classEnums[NUM_CLASSES];
	bool isNaturalCastingClass(Stat classEnum);
	bool isNaturalCastingClass(uint32_t classEnum);
	bool isVancianCastingClass(Stat classEnum);
	D20ClassSystem()
	{
		Stat _charClassEnums[NUM_CLASSES] = 
			{ stat_level_barbarian, stat_level_bard, 
			stat_level_cleric, stat_level_druid, stat_level_fighter, stat_level_monk, stat_level_paladin, stat_level_ranger, stat_level_rogue, stat_level_sorcerer, stat_level_wizard };
		memcpy(classEnums, _charClassEnums, NUM_CLASSES * sizeof(uint32_t));
	};
};

extern D20ClassSystem d20ClassSys;
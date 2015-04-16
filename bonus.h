#pragma once
#include "stdafx.h"
#include "common.h"


struct BonusSystem : AddressTable
{
	uint32_t bonusNonCumulative(BonusList * str378, uint32_t n, int * intOut);
	void initBonusList(BonusList * bonusList);
	int32_t sub_100E65C0(BonusList * bonList);

	int32_t(__cdecl * _sub_100E65C0)(BonusList* bonList);
	BonusSystem();
};

extern BonusSystem bonusSys;

void _initBonusList(BonusList * bonusList);
uint32_t _bonusNonCumulative(BonusList * str378, int n, int * intOut);

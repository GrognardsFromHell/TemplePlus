#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"


struct BonusSystem : AddressTable
{
	uint32_t bonusNonCumulative(BonusList * bonList, uint32_t n, int * intOut);
	uint32_t bonusAddToBonusList(BonusList* bonList, int bonValue, int bonType, unsigned bonMesLineNum);
	uint32_t bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr);
	void initBonusList(BonusList * bonusList);
	int32_t sub_100E65C0(BonusList * bonList);

	int32_t(__cdecl * _sub_100E65C0)(BonusList* bonList);

	MesHandle * bonusMesHandle; //102E45A8
	BonusSystem();
};

extern BonusSystem bonusSys;

void _initBonusList(BonusList * bonusList);
uint32_t _bonusNonCumulative(BonusList * str378, int n, int * idxOut);
uint32_t _bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum);
uint32_t _bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr);
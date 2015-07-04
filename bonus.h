#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"


struct BonusList;

struct BonusSystem : AddressTable
{
	uint32_t isBonusNotMaximal(BonusList * bonList, uint32_t bonIdx, uint32_t * maxBonIdx);
	uint32_t bonusAddToBonusList(BonusList* bonList, int bonValue, int bonType, unsigned bonMesLineNum);
	uint32_t bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr);
	uint32_t bonusCapAdd(BonusList * bonList, int capType, int capValue, uint32_t bonMesLineNum);
	uint32_t bonusCapAddWithDescr(BonusList * bonList, int capType, int capValue, uint32_t bonMesLineNum, char* capDescr);
	uint32_t isBonusCapped(BonusList * bonList, int bonIdx, int * capperIdx);
	void initBonusList(BonusList * bonusList);
	int32_t getOverallBonus(BonusList * bonList);
	uint32_t getNumBonuses(BonusList * bonList);

	void bonusPrintString(BonusList *bonlist, uint32_t bonIdx, int32_t *bonValueOut, char *strBonusOut, char *strCapOut);
	uint32_t zeroBonusSetMeslineNum(BonusList* bonList, uint32_t zeroBonusMesLineNum);
	uint32_t bonusSetOverallCap(uint32_t bonFlags, BonusList* bonList, int32_t newCap, int a4, uint32_t bonMesLineNum, char* capDescr);
	void (__cdecl *_bonusPrintString)(BonusList *bonlist, uint32_t bonIdx, int32_t *bonValueOut, char *strBonusOut, char *strCapOut);
	int32_t(__cdecl * _getOverallBonus)(BonusList* bonList);

	MesHandle * bonusMesHandle; //102E45A8
	MesHandle bonusMesNew;
	BonusSystem();
};

extern BonusSystem bonusSys;

int BonusMesInit();

void _initBonusList(BonusList * bonusList);
uint32_t _isBonusNotMaximal(BonusList * str378, int n, uint32_t * idxOut);
uint32_t _bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum);
uint32_t _bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr);
uint32_t _bonusCapAdd(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum);
uint32_t _bonusCapAddWithDescr(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum, char* capDescr);
uint32_t _isBonusCapped(BonusList* bonList, int bonIdx, int* capperIdx);
uint32_t _getNumBonuses(BonusList * bonList);
uint32_t _zeroBonusSetMeslineNum(BonusList* bonList, uint32_t zeroBonusMesLineNum);
uint32_t _bonusSetOverallCap(uint32_t bonFlags, BonusList* bonList, int32_t newCap, int a4, uint32_t bonMesLineNum, char* capDescr);



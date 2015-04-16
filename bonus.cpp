#include "stdafx.h"
#include "common.h"
#include "bonus.h"
#include "tig/tig_mes.h"


class BonusSysReplacements : public TempleFix
{
	macTempleFix(History System)
	{
		macReplaceFun(100E6490, _bonusNonCumulative)
		macReplaceFun(100E6110, _bonusAddToBonusList)
		macReplaceFun(100E60D0,_initBonusList)
		macReplaceFun(100E6260, _bonusAddToBonusListWithDescr)
	}
} bonusSystemReplacements;




#pragma region Bonus System Implementation
BonusSystem bonusSys;

uint32_t BonusSystem::bonusNonCumulative(BonusList* bonlist, uint32_t bonIdx, int* idxOut)
{
	/*
	returns 0 if bonIdx is the index of the greatest bonus
	returns 0 if the bonus type is stackable
	returns 1 if there's a bigger bonus in the list, and outputs its index into idxOut
	*/
	auto bonValue = bonlist->bonusEntries[bonIdx].bonValue;
	auto bonType = bonlist->bonusEntries[bonIdx].bonType;
	auto bonCount = bonlist->count;
	auto result = 0;
	int iterBonusValue;
	if (bonType != 0 && bonType != 0x8 && bonType != 0x15)
	{
		BonusEntry * iterBonusEntry;
		for (uint32_t i = 0; i < bonCount; i++)
		{
			iterBonusEntry = &bonlist->bonusEntries[i];
			if (i == bonIdx || iterBonusEntry->bonType != bonType) continue;
			iterBonusValue = iterBonusEntry->bonValue;
			if (bonValue <= 0)
			{
				if (iterBonusValue >= bonValue && (iterBonusValue != bonValue || bonIdx <= i)) continue;
			}
			else if (iterBonusValue <= bonValue && (iterBonusValue != bonValue || bonIdx >= i)) continue;
			bonValue = iterBonusValue;
			result = 1;
			*idxOut = i;
		}
	}
	return result;
}

uint32_t BonusSystem::bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum)
{
	MesLine mesLine;
	if (bonList->count >= BonusListMax) return 0;
	mesLine.key = bonMesLineNum;
	mesFuncs.GetLine_Safe(*bonusMesHandle, &mesLine);
	bonList->bonusEntries[bonList->count].bonValue = bonValue;
	bonList->bonusEntries[bonList->count].bonType = bonType;
	bonList->bonusEntries[bonList->count].bonusMesString = (char*)mesLine.value;
	bonList->bonusEntries[bonList->count++].bonusDescr = nullptr;
	return 1;
}

uint32_t BonusSystem::bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char* bonDescr)
{
	if (bonusAddToBonusList(bonList, bonValue, bonType, bonMesLineNum) == 1)
	{
		bonList->bonusEntries[bonList->count - 1].bonusDescr = bonDescr;
		return 1;
	}
	return 0;
}

void BonusSystem::initBonusList(BonusList* bonusList)
{
	bonusList ->count=0	;
	bonusList ->count2=0;
	bonusList ->count3=0;
	bonusList ->minCap = 0x80000001;
	bonusList ->maxCap = 0x7fffFFFF;
	bonusList ->bonFlags = 0;
}

int32_t BonusSystem::sub_100E65C0(BonusList* bonList)
{
	return _sub_100E65C0(bonList);
}

BonusSystem::BonusSystem()
{
	macRebase(bonusMesHandle, 102E45A8)
	macRebase(_sub_100E65C0, 100E65C0)
}
#pragma endregion

#pragma region Replacements
void _initBonusList(BonusList* bonusList)
{
	bonusSys.initBonusList(bonusList);
}

uint32_t _bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum)
{
	return bonusSys.bonusAddToBonusList(bonList, bonValue, bonType, bonMesLineNum);
}

uint32_t _bonusNonCumulative(BonusList* bonlist, int n, int* idxOut)
{
	return bonusSys.bonusNonCumulative(bonlist, n, idxOut);
}

uint32_t _bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr)
{
	return bonusSys.bonusAddToBonusListWithDescr(bonList, bonValue, bonType, bonMesLineNum, bonDescr);
}
#pragma endregion

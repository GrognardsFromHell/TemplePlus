#include "stdafx.h"
#include "common.h"
#include "bonus.h"
#include "tig/tig_mes.h"
#include "util/fixes.h"

int(__cdecl *OrgBonusInit)();

// History System Function Replacements
class BonusSysReplacements : public TempleFix
{
	public: 
		
	void apply() override 
	{
		OrgBonusInit = (int(__cdecl *)())replaceFunction(0x100E5EB0, BonusMesInit);

		replaceFunction(0x100E6490, _isBonusNotMaximal); 
		replaceFunction(0x100E6110, _bonusAddToBonusList); 
		replaceFunction(0x100E60D0, _initBonusList); 
		replaceFunction(0x100E6260, _bonusAddToBonusListWithDescr); 
		replaceFunction(0x100E62A0, _bonusCapAdd); 
		replaceFunction(0x100E6340, _bonusCapAddWithDescr); 
		replaceFunction(0x100E6410, _isBonusCapped); 
		replaceFunction(0x100E63B0, _getNumBonuses); 
		replaceFunction(0x100E6380, _zeroBonusSetMeslineNum); 
		replaceFunction(0x100E61A0, _bonusSetOverallCap); 

		}
} bonusSystemReplacements;




#pragma region Bonus System Implementation
BonusSystem bonusSys;

uint32_t BonusSystem::isBonusNotMaximal(BonusList* bonlist, uint32_t bonIdx, uint32_t* maxBonIdx)
{
	/*
	returns 0 if bonIdx is the index of the greatest bonus of its type (or most negative if it's 0 or less)
	returns 0 if the bonus type is stackable
	returns 1 if there's a bigger bonus in the list, and outputs its index into *maxBonIdx
	*/
	auto bonValue = bonlist->bonusEntries[bonIdx].bonValue;
	auto bonType = bonlist->bonusEntries[bonIdx].bonType;
	auto bonCount = bonlist->bonCount;
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
			*maxBonIdx = i;
		}
	}
	return result;
}

uint32_t BonusSystem::bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum)
{
	MesLine mesLine;
	if (bonList->bonCount >= BonusListMax) return 0;
	mesLine.key = bonMesLineNum;

	if (bonMesLineNum >= 335)
		mesFuncs.GetLine_Safe(bonusMesNew, &mesLine);
	else
		mesFuncs.GetLine_Safe(*bonusMesHandle, &mesLine);
	bonList->bonusEntries[bonList->bonCount].bonValue = bonValue;
	bonList->bonusEntries[bonList->bonCount].bonType = bonType;
	bonList->bonusEntries[bonList->bonCount].bonusMesString = (char*)mesLine.value;
	bonList->bonusEntries[bonList->bonCount++].bonusDescr = nullptr;
	return 1;
}

uint32_t BonusSystem::bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char* bonDescr)
{
	if (bonusAddToBonusList(bonList, bonValue, bonType, bonMesLineNum) == 1)
	{
		bonList->bonusEntries[bonList->bonCount - 1].bonusDescr = bonDescr;
		return 1;
	}
	return 0;
}

uint32_t BonusSystem::bonusCapAdd(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum)
{
	MesLine mesLine;
	if (bonList->bonCapperCount >= 10) {
		auto breakPointDummy = 1;
		if (bonList->bonCapperCount >= BonusListMax) return 0;// bug? there's only 10 slots (this is the original code!)
	} 

	mesLine.key = bonMesLineNum;
	if (bonMesLineNum >= 335)
		mesFuncs.GetLine_Safe(bonusMesNew, &mesLine);
	else
		mesFuncs.GetLine_Safe(*bonusMesHandle, &mesLine);
	bonList->bonCaps[bonList->bonCapperCount].capValue = capValue;
	bonList->bonCaps[bonList->bonCapperCount].bonType = capType;
	bonList->bonCaps[bonList->bonCapperCount].bonCapperString = (char*)mesLine.value;
	bonList->bonCaps[bonList->bonCapperCount++].bonCapDescr = 0;
	return 1;
}

uint32_t BonusSystem::bonusCapAddWithDescr(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum, char* capDescr)
{
	if (bonusCapAdd(bonList, capType, capValue, bonMesLineNum) == 1)
	{
		bonList->bonCaps[bonList->bonCapperCount - 1].bonCapDescr = capDescr;
		return 1;
	}
	return 0;
}

uint32_t BonusSystem::isBonusCapped(BonusList* bonList, int bonIdx, int* capperIdx){
	bool bonTypeMatch = 0;
	int32_t capValueLowest = 255;
	int32_t bonValue = bonList->bonusEntries[bonIdx].bonValue;
	uint32_t bonType = bonList->bonusEntries[bonIdx].bonType;
	uint32_t bonCapCount = bonList->bonCapperCount;
	for (uint32_t i = 0; i < bonCapCount; i++)
	{
		uint32_t bonCapType = bonList->bonCaps[i].bonType;
		if (!bonCapType || bonCapType == bonType)
		{
			int32_t capValue = abs(bonList->bonCaps[i].capValue);
			if (capValue < capValueLowest)
			{
				capValueLowest = capValue;
				*capperIdx = i;
				bonTypeMatch = 1;
			}
		}
	}
	if (capValueLowest > bonValue) return 0; 
	return bonTypeMatch;
}

void BonusSystem::initBonusList(BonusList* bonusList){
	bonusList ->bonCount=0	;
	bonusList ->bonCapperCount=0;
	bonusList ->zeroBonusCount=0;
	bonusList ->overallCapLow.bonValue = 0x80000001;
	bonusList ->overallCapHigh.bonValue = 0x7fffFFFF;
	bonusList ->bonFlags = 0;
}

int32_t BonusSystem::getOverallBonus(BonusList* bonList)
{
	return _getOverallBonus(bonList);
}

uint32_t BonusSystem::getNumBonuses(BonusList* bonList)
{
	return bonList->bonCount + bonList->zeroBonusCount;
}

void BonusSystem::bonusPrintString(BonusList* bonlist, uint32_t bonIdx, int32_t* bonValueOut, char* strBonusOut, char* strCapOut)
{
	_bonusPrintString(bonlist, bonIdx, bonValueOut, strBonusOut, strCapOut);
}

uint32_t BonusSystem::zeroBonusSetMeslineNum(BonusList* bonList, uint32_t zeroBonusMesLineNum)
{
	uint32_t count = bonList->zeroBonusCount;
	if (count >= 10) return 0;
	bonList->zeroBonusReasonMesLine[count] = zeroBonusMesLineNum;
	++bonList->zeroBonusCount;
	return 1;
}

uint32_t BonusSystem::bonusSetOverallCap(uint32_t bonFlags, BonusList* bonList, int32_t newCap, int newCapType, uint32_t bonMesLineNum, char* capDescr){
	return bonList->SetOverallCap(bonFlags, newCap, newCapType, bonMesLineNum, capDescr);
}

BonusSystem::BonusSystem()
{
	rebase(_bonusPrintString,0x100E6740); 
	rebase(bonusMesHandle,0x102E45A8); 
	rebase(_getOverallBonus,0x100E65C0); 
}
#pragma endregion

#pragma region Replacements


int BonusMesInit()
{
	mesFuncs.Open("tpmes\\bonus.mes", &bonusSys.bonusMesNew);
	return OrgBonusInit();
}
void _initBonusList(BonusList* bonusList)
{
	bonusSys.initBonusList(bonusList);
}

uint32_t _bonusAddToBonusList(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum)
{
	return bonusSys.bonusAddToBonusList(bonList, bonValue, bonType, bonMesLineNum);
}

uint32_t _isBonusNotMaximal(BonusList* bonlist, int n, uint32_t* idxOut)
{
	return bonusSys.isBonusNotMaximal(bonlist, n, idxOut);
}

uint32_t _bonusAddToBonusListWithDescr(BonusList* bonList, int32_t bonValue, int32_t bonType, uint32_t bonMesLineNum, char * bonDescr)
{
	return bonusSys.bonusAddToBonusListWithDescr(bonList, bonValue, bonType, bonMesLineNum, bonDescr);
}


uint32_t _bonusCapAdd(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum)
{
	return bonusSys.bonusCapAdd(bonList, capType, capValue, bonMesLineNum);
}

uint32_t _bonusCapAddWithDescr(BonusList* bonList, int capType, int capValue, uint32_t bonMesLineNum, char* capDescr)
{
	return bonusSys.bonusCapAddWithDescr(bonList, capType, capValue, bonMesLineNum, capDescr);
}

uint32_t _isBonusCapped(BonusList* bonList, int bonIdx, int* capperIdx)
{
	return bonusSys.isBonusCapped(bonList, bonIdx, capperIdx);
}

uint32_t _getNumBonuses(BonusList* bonList)
{
	return bonusSys.getNumBonuses(bonList);
}

uint32_t _zeroBonusSetMeslineNum(BonusList* bonList, uint32_t zeroBonusMesLineNum)
{
	return bonusSys.zeroBonusSetMeslineNum(bonList, zeroBonusMesLineNum);
}

uint32_t _bonusSetOverallCap(uint32_t bonFlags, BonusList* bonList, int32_t newCap, int a4, uint32_t bonMesLineNum, char* capDescr)
{
	return bonusSys.bonusSetOverallCap(bonFlags, bonList, newCap, a4, bonMesLineNum, capDescr);
}
#pragma endregion

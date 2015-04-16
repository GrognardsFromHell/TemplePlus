#include "stdafx.h"
#include "common.h"
#include "bonus.h"




class HistSysReplacements : public TempleFix
{
	macTempleFix(History System)
	{
		macReplaceFun(100E6490, _bonusNonCumulative)
	}
} bonusSystemReplacements;




#pragma region Bonus System Implementation
BonusSystem bonusSys;

uint32_t BonusSystem::bonusNonCumulative(BonusList* str378, uint32_t n, int* idxOut)
{
	/*
	returns 0 if n is the index of the greatest bonus
	returns 0 if the bonus type is stackable
	returns 1 if there's a bigger bonus in the list, and outputs its index into idxOut
	*/
	auto value = str378->bonusEntries[n].value;
	auto v4 = str378->bonusEntries[n].bonusType_maybe;
	auto count = str378->count;
	auto result = 0;
	int iterBonusValue;
	if (v4 != 0 && v4 != 0x8 && v4 != 0x15)
	{
		BonusEntry * iterBonusEntry;
		for (uint32_t i = 0; i < count; i++)
		{
			iterBonusEntry = &str378->bonusEntries[i];
			if (i == n || iterBonusEntry->bonusType_maybe != v4) continue;
			iterBonusValue = iterBonusEntry->value;
			if (value <= 0)
			{
				if (iterBonusValue >= value && (iterBonusValue != value || n <= i)) continue;
			}
			else if (iterBonusValue <= value && (iterBonusValue != value || n >= i)) continue;
			value = iterBonusValue;
			result = 1;
			*idxOut = i;
		}
	}
	return result;
}

void BonusSystem::initBonusList(BonusList* bonusList)
{
	bonusList ->count=0	;
	bonusList ->count2=0;
	bonusList ->count3=0;
	bonusList ->minSthg = 0x80000001;
	bonusList ->maxSthg = 0x7fffFFFF;
	bonusList ->flags = 0;
}

int32_t BonusSystem::sub_100E65C0(BonusList* bonList)
{
	return _sub_100E65C0(bonList);
}

BonusSystem::BonusSystem()
{
	macRebase(_sub_100E65C0, 100E65C0)
}
#pragma endregion

#pragma region Replacements
void _initBonusList(BonusList* bonusList)
{
	bonusSys.initBonusList(bonusList);
}

uint32_t _bonusNonCumulative(BonusList* str378, int n, int* intOut)
{
	return bonusSys.bonusNonCumulative(str378, n, intOut);
}

#pragma endregion

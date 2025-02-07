
#include "stdafx.h"
#include "dice.h"
#include <temple/dll.h>

static struct DiceAddresses : temple::AddressTable {

	int (__cdecl *DiceRoller)(int nNum_Die, int nDie_Type, int nBonus);
	
	int (__cdecl *RandomIntRange)(int from, int to);

	DiceAddresses() {
		rebase(DiceRoller, 0x10038B60);
		rebase(RandomIntRange, 0x10038DF0);
	}
} addresses;

int Dice::Roll(int count, int sides, int modifier) {
	return addresses.DiceRoller(count, sides, modifier);
}

int Dice::Roll() {
	return Roll(mCount, mSides, mModifier);
}

bool Dice::Parse(const char* diceStr, int& count, int& sides, int& modifier) {
	modifier = 0;
	count = atoi(diceStr);
	auto tmp = strchr(diceStr, 'd');
	if (tmp) {
		sides = atoi(tmp + 1);
		tmp = strchr(diceStr, '-');
		if (!tmp) {
			tmp = strchr(diceStr, '+');
		}
		if (tmp) {
			modifier = atoi(tmp);
		}
		return true;
	}
	return false;
}

Dice Dice::IncreaseWeaponSize(int numIncreases) const
{
	auto diceCount = mCount;
	auto diceSide = mSides;
	auto diceMod = mModifier;

	for (int i = 0; i < numIncreases; ++i) {
		switch (diceSide)
		{
		case 2:
			diceSide = 3;
			break;
		case 3:
			diceSide = 4;
			break;
		case 4:
			diceSide = 6;
			break;
		case 6:
			if (diceCount == 1)
				diceSide = 8;
			else if (diceCount <= 3)
				diceCount++;
			else
				diceCount += 2;
			break;
		case 8:
			if (diceCount == 1) {
				diceCount = 2;
				diceSide = 6;
			}
			else if (diceCount <= 3)
			{
				diceCount++;
			}
			else if (diceCount <= 6) {
				diceCount += 2;
			}
			else
				diceCount += 4;
			break;
		case 10:
			diceCount *= 2;
			diceSide = 8;
			break;
		case 12:
			diceCount = 3;
			diceSide = 6;
			break;
		default:
			break;
		}
	}

	return Dice(diceCount, diceSide, diceMod);
}


int RandomIntRange(int from, int to) {
	return addresses.RandomIntRange(from, to);
}

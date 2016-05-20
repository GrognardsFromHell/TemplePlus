#pragma once
#include "dice.h"

struct PoisonSpec{
	char dc;
	char immNumDie;
	char immDieType;
	char immDieBonus;
	int immediateEffect; // -7 does direct damage, -8 causes paralysis, -10 nothing

	PackedDie immSecDice;
	int immediateSecondEffect;

	PackedDie delayedDice;
	int delayedEffect;

	PackedDie delayedSecDice;
	int delayedSecondEffect;

};

//const int testSizeOfPoisonSpec = sizeof PoisonSpec; // should be 32 (0x20)




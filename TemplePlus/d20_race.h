#pragma once
#include "d20_defs.h"
#include "ui/ui_chargen.h"
#include "critter.h"


class RaceSpec;

class D20RaceSys
{
public:
	int vanillaRaceEnums[VANILLA_NUM_RACES];

	D20RaceSys();
	int GetStatModifier(Race race, int stat); // e.g. +2 CON for Dwarves

	HairStyleRace GetHairStyle(Race race); // get racial hair style from among those available in ToEE
protected:
	RaceSpec & GetRaceSpec(Race race);
	std::map<Race, RaceSpec> mRaceSpecs;
};


extern D20RaceSys d20RaceSys;

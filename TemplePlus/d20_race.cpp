#include "stdafx.h"
#include "d20_race.h"
#include "temple_enums.h"

class RaceSpec
{
public:
	std::vector<int> statModifiers;

	RaceSpec(){
		statModifiers = { 0,0,0,0,0,0 };
	}
};

D20RaceSys d20RaceSys;

D20RaceSys::D20RaceSys()
{
	int _charRaceEnums[VANILLA_NUM_RACES] =
	{ Race::race_human, Race::race_dwarf, Race::race_elf,
		Race::race_gnome, Race::race_halfelf, Race::race_half_orc,
		Race::race_halfling	 };
	memcpy(vanillaRaceEnums, _charRaceEnums, VANILLA_NUM_RACES * sizeof(uint32_t));

	for (auto it:vanillaRaceEnums){
		mRaceSpecs[(Race)it] = RaceSpec();
	}

	mRaceSpecs[Race::race_dwarf].statModifiers[Stat::stat_constitution] = 2;
	mRaceSpecs[Race::race_dwarf].statModifiers[Stat::stat_charisma] = -2;

	mRaceSpecs[Race::race_elf].statModifiers[Stat::stat_constitution] = -2;
	mRaceSpecs[Race::race_elf].statModifiers[Stat::stat_dexterity] = 2;

	mRaceSpecs[Race::race_gnome].statModifiers[Stat::stat_constitution] = 2;
	mRaceSpecs[Race::race_gnome].statModifiers[Stat::stat_strength] = -2;

	mRaceSpecs[Race::race_half_orc].statModifiers[Stat::stat_charisma] = -2;
	mRaceSpecs[Race::race_half_orc].statModifiers[Stat::stat_strength] = 2;

	mRaceSpecs[Race::race_halfling].statModifiers[Stat::stat_dexterity] = 2;
	mRaceSpecs[Race::race_halfling].statModifiers[Stat::stat_strength] = -2;

}

int D20RaceSys::GetStatModifier(Race race, int stat) {
	auto &spec = GetRaceSpec(race);
	if (stat <= stat_charisma)
		return spec.statModifiers[stat];
	return 0;
}

RaceSpec& D20RaceSys::GetRaceSpec(Race race){
	if (mRaceSpecs.find(race) != mRaceSpecs.end()) {
		return mRaceSpecs[race];
	}
	logger->error(fmt::format("Bad race input {}, returning human instead", (int)race).c_str());
	return mRaceSpecs[Race::race_human];
}

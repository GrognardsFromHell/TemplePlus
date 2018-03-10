#include "stdafx.h"
#include "d20_race.h"
#include "temple_enums.h"

D20RaceSys d20RaceSys;

D20RaceSys::D20RaceSys()
{
	int _charRaceEnums[VANILLA_NUM_RACES] =
	{ Race::race_human, Race::race_dwarf, Race::race_elf,
		Race::race_gnome, Race::race_halfelf, Race::race_half_orc,
		Race::race_halfling	 };
	memcpy(vanillaRaceEnums, _charRaceEnums, VANILLA_NUM_RACES * sizeof(uint32_t));
}

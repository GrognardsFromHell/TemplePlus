
#pragma once

#include "temple_enums.h"

enum class StatType {
	Abilities = 0,
	Level = 1,
	HitPoints = 2,
	Combat = 3,
	Money = 4,
	AbilityMods = 5,
	Speed = 6,
	Feat = 7,
	Race = 8,
	Load = 9,
	SavingThrows = 10,
	// Unknown = 11
	Other = 12
};

class D20StatsSystem {
public:
	
private:

	static StatType GetType(Stat stat);
};


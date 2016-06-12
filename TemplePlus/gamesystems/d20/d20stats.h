
#pragma once

#include "temple_enums.h"
#include <tig/tig_mes.h>
#include <map>

struct GameSystemConf;
struct objHndl;

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
	friend class StatSystem;
public:
	
	const char* GetStatName(Stat stat) const;
	const char* GetStatEnumString(Stat stat) const; // not really needed - just used for the python layer (replaced in constants.py)
	const char* GetStatRulesString(Stat stat) const;
	const char* GetClassShortDesc(Stat stat) const;
	const char* GetCannotPickClassHelp(Stat stat) const;

	int GetLevelStat(const objHndl &handle, Stat stat) const;

private:

	void Init(const GameSystemConf& conf);
	bool isEditor = false;
	MesHandle statRules =0;
	MesHandle statRulesExt = 0;
	MesHandle statMes =0;
	MesHandle statMesExt = 0;
	MesHandle statEnum =0;
	

	const char* statMesStrings[_stat_count] = {nullptr,};
	const char* statEnumStrings[_stat_count] = { nullptr, };
	const char* statRulesStrings[_stat_count] = { nullptr, };
	const char* statShortNameStrings[_stat_count] = { nullptr, };
	std::map<int, std::string> cannotPickClassStr;
	static StatType GetType(Stat stat);
};

extern D20StatsSystem d20Stats;



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
	Physical = 3,
	Money = 4,
	AbilityMods = 5,
	Speed = 6,
	Feat = 7,
	Race = 8,
	Load = 9,
	SavingThrows = 10,
	SpellCasting = 11, // (originally missing, this was probably related to the stat_caster_level etc stats)
	Other = 12,
	Psi = 13
};

class D20StatsSystem {
	friend class StatSystem;
	friend class D20StatsHooks;
public:
	
	const char* GetStatName(Stat stat) const;
	const char* GetStatShortName(Stat stat) const;
	const char* GetStatEnumString(Stat stat) const; // not really needed - just used for the python layer (replaced in constants.py)
	const char* GetStatRulesString(Stat stat) const;
	const char* GetClassShortDesc(Stat stat) const;
	const char* GetAlignmentName(Alignment alignment);
	const char* GetRaceName(Race race);
	const char* GetRaceShortDesc(Race race);
	const char* GetMonsterSubcategoryName(int monsterSubcat);
	const char* GetMonsterCategoryName(int monsterCat);
	const char* GetGenderName(int genderId);
	const char* GetCannotPickClassHelp(Stat stat) const;

	int GetValue(const objHndl &handle, Stat stat, int statArg = -1) const;
	int GetBaseValue(const objHndl &handle, Stat stat, int statArg = -1) const;
	int GetLevelStat(const objHndl &handle, Stat stat) const;
	int GetSpellCastingStat(const objHndl &handle, Stat stat, int statArg) const;
	int GetBaseAttackBonus(const objHndl &handle, Stat classLeveled) const;
	int GetPsiStat(const objHndl &handle, Stat stat, int statArg = -1) const;
	int GetPsiStatBase(const objHndl &handle, Stat stat, int statArg = -1) const;
	int GetPhysicalStatBase(const objHndl &handle, Stat stat) const;
	int GetPhysicalStatLevel(const objHndl &handle, Stat stat) const;
	bool AlignmentsUnopposed(Alignment a, Alignment b, bool strictCheck = false);
	


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


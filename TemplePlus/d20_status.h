#pragma once
#include "common.h"
#include <map>
struct Dispatcher;

class D20StatusSystem
{
public:
	void initRace(objHndl objHnd);
	void initClass(objHndl objHnd);
	void D20StatusInit(objHndl objHnd);
	void D20StatusRefresh(objHndl objHnd);
	void initDomains(objHndl objHnd);
	void initFeats(objHndl objHnd);
	void initItemConditions(objHndl objHnd);
	void D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher *dispatcher);

	std::map<Stat, std::string> classCondMap = {
		{ Stat::stat_level_barbarian,"Barbarian" },
		{ Stat::stat_level_bard,"Bard" },
		{ Stat::stat_level_cleric,"Cleric" },
		{ Stat::stat_level_druid,"Druid" },
		{ Stat::stat_level_fighter,"Fighter" },
		{ Stat::stat_level_monk,"Monk" },
		{ Stat::stat_level_paladin,"Paladin" },
		{ Stat::stat_level_ranger,"Ranger" },
		{ Stat::stat_level_rogue,"Rogue" },
		{ Stat::stat_level_sorcerer,"Sorcerer" },
		{ Stat::stat_level_wizard,"Wizard" },
	};
};

extern D20StatusSystem d20StatusSys;
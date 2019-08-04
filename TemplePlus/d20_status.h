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
	void initDomain(Dispatcher * dispatcher, uint32_t domain);
	void initDomains(objHndl objHnd);
	void initFeats(objHndl objHnd);
	void initItemConditions(objHndl objHnd);
	void InitFromItemConditionFields(Dispatcher* dispatcher, objHndl item, int invIdx); // inits conditions for the wearer from the item. Note: args[2] is set to be the inventory index here!
	void D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher *dispatcher);

	// mapping of class enum to condition name. Gets updated from python specs.
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

	std::map<Race, std::string> raceCondMap = {
	{ Race::race_human,"Human" },
	{ Race::race_dwarf,"Dwarf" },
	{ Race::race_elf,"Elf" },
	{ Race::race_gnome,"Gnome" },
	{ Race::race_half_elf,"Half-Elf" },
	{ Race::race_half_orc,"Hal-Orc" },
	{ Race::race_halfling,"Halfling" },
	};
};

extern D20StatusSystem d20StatusSys;
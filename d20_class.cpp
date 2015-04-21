#include "stdafx.h"
#include "d20_class.h"

D20ClassSystem d20ClassSys;

bool D20ClassSystem::isNaturalCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_bard || classEnum == stat_level_sorcerer) return 1;
	return 0;
}

bool D20ClassSystem::isNaturalCastingClass(uint32_t classEnum)
{
	Stat castedClassEnum = static_cast<Stat>(classEnum);
	if (classEnum == stat_level_bard || classEnum == stat_level_sorcerer) return 1;
	return 0;
}

bool D20ClassSystem::isVancianCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_cleric || classEnum == stat_level_druid || classEnum == stat_level_paladin || classEnum == stat_level_ranger || classEnum == stat_level_wizard) return 1;
	return 0;
}
from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	spellcraft_level = attachee.skill_ranks_get(skill_spellcraft)
	if spellcraft_level < 4:
		return 0
	return 1
from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	
	paladinLevel = attachee.stat_level_get(stat_level_paladin)
	if classLeveled == stat_level_paladin:
		paladinLevel = paladinLevel + 1
	if paladinLevel < 4 or not attachee.divine_spell_level_can_cast() > 0:
		return 0

	return 1

from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	warmageLevel = attachee.stat_level_get(stat_level_warmage)
	if classLeveled == stat_level_warmage:
		warmageLevel = warmageLevel + 1
	if warmageLevel < 4:
		return 0

	return 1

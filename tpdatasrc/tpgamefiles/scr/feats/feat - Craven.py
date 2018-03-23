from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Sneak Attack Check
	if not attachee.has_feat(feat_sneak_attack):
		return 0
	if attachee.stat_level_get(stat_level_paladin) > 1: #workaround until I figure out how to check for immunity to fear
		return 0
	return 1

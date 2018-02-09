from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Barbarian Rage Check
	if not attachee.has_feat(feat_barbarian_rage):
		return 0

	return 1

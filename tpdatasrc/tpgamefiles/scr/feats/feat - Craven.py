from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Sneak Attack Check
	if not attachee.has_feat(feat_sneak_attack):
		return 0
	return 1

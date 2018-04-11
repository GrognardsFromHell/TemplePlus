from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):

	if attachee.has_feat(feat_sneak_attack):
		return 1
	
	return 0

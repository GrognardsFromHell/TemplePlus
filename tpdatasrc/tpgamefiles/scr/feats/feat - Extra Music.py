from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Bardic Music Check
	if not attachee.has_feat(feat_bardic_music):
		return 0
	return 1

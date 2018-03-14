from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Turn or rebuke undead feat
	if (attachee.has_feat(feat_turn_undead) or attachee.has_feat(feat_rebuke_undead)):
		return 1
	
	return 0

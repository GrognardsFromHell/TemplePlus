from toee import *


def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Req 1, turn undead feat
	if not (attachee.has_feat(feat_turn_undead) or attachee.has_feat(feat_rebuke_undead)):
		return 0
	
	#Req2, Divine Caster Level 5
	if attachee.highest_divine_caster_level >= 5:
		return 1
		
	return 0

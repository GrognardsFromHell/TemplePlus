from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Turn or rebuke undead feat
	if (char_editor.has_feat(feat_turn_undead) or char_editor.has_feat(feat_rebuke_undead)):
		return 1
	
	return 0

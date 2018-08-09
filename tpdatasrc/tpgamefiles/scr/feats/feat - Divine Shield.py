from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Req 1, turn undead feat
	if not (char_editor.has_feat(feat_turn_undead) or char_editor.has_feat(feat_rebuke_undead)):
		return 0
		
	#Req2, shield proficency
	if not char_editor.has_feat(feat_shield_proficiency):
		return 0
		
	return 1

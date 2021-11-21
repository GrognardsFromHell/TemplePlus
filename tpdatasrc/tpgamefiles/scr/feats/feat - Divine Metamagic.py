from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#First requirement turn or rebuke wndead.  Second requirement is enforced for the child feats.
	if not (char_editor.has_feat(feat_turn_undead) or char_editor.has_feat(feat_rebuke_undead)):
		return 0
		
	return 1

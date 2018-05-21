from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Shield proficency required
	if not char_editor.has_feat(feat_shield_proficiency):
		return 0
		
	return 1

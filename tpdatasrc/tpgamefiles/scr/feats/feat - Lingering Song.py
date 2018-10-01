from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Bardic Music Check
	if not char_editor.has_feat(feat_bardic_music):
		return 0

	return 1

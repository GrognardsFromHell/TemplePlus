from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Barbarian Rage Check
	if not char_editor.has_feat(feat_barbarian_rage):
		return 0

	return 1

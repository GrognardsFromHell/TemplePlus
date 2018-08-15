from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	#BAB +6 enforced by engine
	
	#Must have sneak attack
	if not char_editor.has_feat(feat_sneak_attack):
		return 0

	return 1

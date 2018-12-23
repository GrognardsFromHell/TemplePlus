from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Wild Shape Check
	if not char_editor.has_feat(feat_wild_shape):
		return 0

	return 1

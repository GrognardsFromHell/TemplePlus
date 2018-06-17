from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Wildshape feat
	if not (char_editor.has_feat(feat_wild_shape)):
		return 0
	
	#Dextarity 13 or greater
	if char_editor.stat_level_get(stat_dexterity) < 13:
		return 0
	
	return 1

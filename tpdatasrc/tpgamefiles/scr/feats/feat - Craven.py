from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Sneak Attack Check
	if not char_editor.has_feat(feat_sneak_attack):
		return 0
	if char_editor.stat_level_get(stat_level_paladin) > 1: #workaround until I figure out how to check for immunity to fear
		return 0
	return 1

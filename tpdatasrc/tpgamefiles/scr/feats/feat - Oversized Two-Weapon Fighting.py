from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Str requirement enforced by the engine
	
	if not char_editor.has_feat(feat_two_weapon_fighting) and not char_editor.has_feat(feat_two_weapon_fighting_ranger):
		return 0
		
	return 1

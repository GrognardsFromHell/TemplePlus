from toee import *
import char_editor

#+1 BAB enforced in the in the .txt file, weapon focus already requires proficiency so I am not enforcing it

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	if char_editor.has_feat(feat_weapon_focus_light_crossbow):
		return 1
	
	if char_editor.has_feat(feat_weapon_focus_heavy_crossbow):
		return 1
	
	if char_editor.has_feat(feat_weapon_focus_hand_crossbow):
		return 1
		
	return 0
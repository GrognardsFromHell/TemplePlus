from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Dex and BAB requirements enforced by the engine
	
	if not char_editor.has_feat(feat_two_weapon_defense):
		return 0
	
	if not char_editor.has_feat(feat_two_weapon_fighting) and not char_editor.has_feat(feat_two_weapon_fighting_ranger):
		return 0
	
	if not char_editor.has_feat("Improved Two-Weapon Defense"):
		return 0
		
	return 1

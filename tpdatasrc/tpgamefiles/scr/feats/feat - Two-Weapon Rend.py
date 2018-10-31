from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#BAB +11 or greater enforced by the engine
	
	#Must have two-weapon fighting
	if not char_editor.has_feat(feat_two_weapon_fighting) and not char_editor.has_feat(feat_two_weapon_fighting_ranger):
		return 0
	
	#Dex 15+
	if char_editor.stat_level_get(stat_dexterity) < 15:
		return 0
		
	return 1

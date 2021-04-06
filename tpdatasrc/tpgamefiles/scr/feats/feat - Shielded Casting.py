from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):

	if not char_editor.has_feat(feat_shield_proficiency):
		return 0
	
	if not char_editor.has_feat(feat_combat_casting):
		return 0
		
	concentration_level = char_editor.skill_ranks_get(skill_concentration)
	if concentration_level < 5:
		return 0
	
	return 1

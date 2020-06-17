from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Perform 4 ranks
	spellcraft_level = char_editor.skill_ranks_get(skill_spellcraft)
	if spellcraft_level < 4:
		return 0
	
	#Spellcraft 4 ranks
	perform_level = char_editor.skill_ranks_get(skill_perform)
	if perform_level < 4:
		return 0
	
	#bardic music class feature
	if not char_editor.has_feat(feat_bardic_music):
		return 0
		
	return 1

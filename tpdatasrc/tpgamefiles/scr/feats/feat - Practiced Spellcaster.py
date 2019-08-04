from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	spellcraft_level = char_editor.skill_ranks_get(skill_spellcraft)
	if spellcraft_level < 4:
		return 0
	return 1
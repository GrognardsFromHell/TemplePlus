from toee import *
import char_editor

	
def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	#Check for 4 ranks in heal
	heal_level = char_editor.skill_ranks_get(skill_heal)
	if heal_level < 4:
		return 0
	return 1

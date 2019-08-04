from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Requirement is an animal companion
	if (char_editor.stat_level_get(stat_level_druid) < 1) and (char_editor.stat_level_get(stat_level_ranger) < 4):
		return 0

	return 1

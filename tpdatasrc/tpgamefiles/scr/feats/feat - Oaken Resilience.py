from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Wild shape to plant form is the requirement which only a 12th level druid gets
	if char_editor.stat_level_get(stat_level_druid) < 12:
		return 0
		
	return 1

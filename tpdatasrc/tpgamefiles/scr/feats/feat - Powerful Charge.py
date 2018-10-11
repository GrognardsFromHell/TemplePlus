from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#BAB +1 or greater enforced by the engine
	
	#Must be medium size or larger
	size = attachee.stat_level_get(stat_size)
	
	#Size must be medium or larger
	if (size < STAT_SIZE_MEDIUM):
		return 0
		
	return 1

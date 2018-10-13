from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#BAB +4 or greater enforced by the engine
	
	#Must be medium size or larger
	size = attachee.stat_level_get(stat_size)
	
	if (size < STAT_SIZE_MEDIUM):
		return 0
		
	#Powerful Charge Check
	if not char_editor.has_feat("Powerful Charge"):
		return 0
		
	return 1

from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#BAB +4 or greater enforced by the engine
	
	#Must be medium size or larger (this should get the base size)
	size = attachee.obj_get_int(obj_f_size)
	
	if (size < STAT_SIZE_MEDIUM):
		return 0
		
	#Powerful Charge Check
	if not char_editor.has_feat("Powerful Charge"):
		return 0
		
	return 1

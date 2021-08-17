from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	#Str 13 or greater and has power attack are enforced in the engine

	if char_editor.has_feat("Brutal Throw") == 0:
		return 0
	
	return 1

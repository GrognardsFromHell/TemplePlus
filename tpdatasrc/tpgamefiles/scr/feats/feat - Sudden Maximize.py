from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#Must have a metamagic feat
	if not char_editor.has_metamagic_feat():
		return 0

	return 1

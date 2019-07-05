from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):

	#Check for a level in any class that allows casting in some armor without arcane failure
	if char_editor.stat_level_get(stat_level_assassin) > 0:
		return 1
		
	if char_editor.stat_level_get(stat_level_bard) > 0:
		return 1
		
	if char_editor.stat_level_get(stat_level_warmage) > 0:
		return 1
		
	if char_editor.stat_level_get(stat_level_beguiler) > 0:
		return 1
	
	return 0

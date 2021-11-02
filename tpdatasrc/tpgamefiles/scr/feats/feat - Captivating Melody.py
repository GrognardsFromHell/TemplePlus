from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	
	#Bardic Music Check & Can Cast Aracne spells (require a bard level for now)
	bardLevel = attachee.stat_level_get(stat_level_bard)
	if classLeveled == stat_level_bard:
		bardLevel = bardLevel + 1
		
	if bardLevel < 1:
		return 0

	return 1

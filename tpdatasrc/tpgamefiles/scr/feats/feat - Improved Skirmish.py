from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	
	#Skirmish +2D6, +1 AC
	scoutLevel = attachee.stat_level_get(stat_level_scout)
	if classLeveled == stat_level_scout:
		scoutLevel = scoutLevel + 1
	if scoutLevel < 5:
		return 0
	
	return 1

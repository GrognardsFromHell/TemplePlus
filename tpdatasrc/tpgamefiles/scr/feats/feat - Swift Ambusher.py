from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):

	#Sneak Attack Bonus +1D6
	if attachee.d20_query_with_data("Sneak Attack Dice", classLeveled) < 1:
		if classLeveled != stat_level_rogue:
			return 0
	
	#Skirmish +1D6
	scoutLevel = attachee.stat_level_get(stat_level_scout)
	if classLeveled == stat_level_scout:
		scoutLevel = scoutLevel + 1
	if scoutLevel < 1:
		return 0
	
	return 1

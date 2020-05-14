from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	
	#Sneak Attack Bonus +2D6
	if attachee.d20_query_with_data("Sneak Attack Dice", classLeveled) < 2:
		return 0
	
	#Grace +1 requirement
	swashBucklerLevel = attachee.stat_level_get(stat_level_swashbuckler)
	if classLeveled == stat_level_swashbuckler:
		swashBucklerLevel = swashBucklerLevel + 1
	if swashBucklerLevel < 2:
		return 0
	
	return 1

from toee import *
import char_editor

def CheckPrereq(attachee, classLeveled, abilityScoreRaised):
	
	#Skirmish +2D6, +1 AC
	scoutLevel = attachee.stat_level_get(stat_level_scout)
	if classLeveled == stat_level_scout:
		scoutLevel = scoutLevel + 1
	
	#Handle special feats that allow other class levels to count toward skirmish ability
	if char_editor.has_feat("Swift Ambusher"):
		rogueLevel = attachee.stat_level_get(stat_level_rogue)
		if classLeveled == stat_level_rogue:
			rogueLevel = rogueLevel + 1
		scoutLevel = scoutLevel + rogueLevel
		
	if char_editor.has_feat("Swift Hunter"):
		rangerLevel = attachee.stat_level_get(stat_level_ranger)
		if classLeveled == stat_level_ranger:
			rangerLevel = rangerLevel + 1
		scoutLevel = scoutLevel + rangerLevel
		
	if scoutLevel < 5:
		return 0
	
	return 1

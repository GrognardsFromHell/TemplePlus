from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	
	#BAB +2 inforaced by the DLL
	
	#Barbarian Rage Check
	if not char_editor.has_feat(feat_stunning_fist):
		return 0
	
	#Wisdom 15 or greater
	if char_editor.stat_level_get(stat_wisdom) < 15:
		return 0

	return 1

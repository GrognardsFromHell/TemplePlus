from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	#Return zero if base attack bonus is too low
	if attachee.get_base_attack_bonus() < 4:
		return 0
	
	#Paladin Smite Evil Check
	if char_editor.has_feat(feat_smite_evil):
		return 1
	
	#Destruction Domain Check
	domain_1 = attachee.obj_get_int(obj_f_critter_domain_1)
	domain_2 = attachee.obj_get_int(obj_f_critter_domain_2)
	if domain_1 == destruction or domain_2 == destruction:
		return 1
	
	#Blackguard Smite for level 2 or greater black guards
	if char_editor.stat_level_get(stat_level_blackguard) >= 2:
		return 1
	
	#Blackguard Smite for level 1 with at least one paladin level
	if char_editor.stat_level_get(stat_level_blackguard) == 1 and char_editor.stat_level_get(stat_level_paladin) >= 1:
		return 1
	
	return 0

from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):

	#Return zero if base attack bonus is too low
	if attachee.get_base_attack_bonus() < 4:
		return 0
	
	if char_editor.has_feat(feat_power_attack) == 0:
		return 0
	
	#Check for any favored enemy feat
	for i in range (feat_favored_enemy_aberration , feat_favored_enemy_humanoid_human):
		if char_editor.has_feat(i):
			return 1
	
	return 0

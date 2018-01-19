from toee import *

featDamType = D20DT_SLASHING

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
	knownFeats = attachee.feats
	hasWeapSpec = 0
	hasWeapFocus = 0

	for ft in knownFeats:
		if hasWeapSpec == 0 and feat_weapon_specialization_gauntlet <= ft <= feat_weapon_specialization_grapple:
			weap_type = game.get_weapon_type_for_feat(ft)
			if wt_gauntlet <= weap_type < 1000:
				dam_type = game.get_weapon_damage_type(weap_type)
				if game.damage_type_match(dam_type, featDamType):
					hasWeapSpec = 1
		if hasWeapFocus == 0 and feat_weapon_focus_gauntlet <= ft <= feat_weapon_focus_repeating_crossbow:
			weap_type = game.get_weapon_type_for_feat(ft)
			if wt_gauntlet <= weap_type < 1000:
				dam_type = game.get_weapon_damage_type(weap_type)
				if game.damage_type_match(dam_type, featDamType):
					hasWeapFocus = 1
	if hasWeapFocus and hasWeapSpec:
		return 1
	return 0
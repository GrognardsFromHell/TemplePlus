from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Fighter"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_FIGHTERS"

classEnum = stat_level_fighter

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all)
}

bonus_feats = [feat_blind_fight, feat_power_attack, feat_cleave, feat_great_cleave, feat_combat_expertise, 
feat_improved_disarm, feat_improved_feint, feat_improved_trip, feat_whirlwind_attack,
feat_combat_reflexes, feat_dodge, feat_mobility, feat_superior_expertise, 
feat_martial_weapon_proficiency_head, feat_improved_critical_head, feat_improved_initiative,
feat_improved_unarmed_strike, feat_deflect_arrows, feat_improved_grapple, 
feat_improved_overrun, feat_improved_shield_bash , feat_improved_two_weapon_fighting,
feat_weapon_finesse_head, feat_greater_two_weapon_fighting, feat_sunder,
feat_two_weapon_defense, feat_two_weapon_fighting,  feat_improved_precise_shot,
feat_trample, feat_stunning_fist, feat_spring_attack, feat_spirited_charge,
feat_snatch_arrows, feat_shot_on_the_run, feat_ride_by_attack, feat_rapid_reload,
feat_rapid_shot, feat_quick_draw, feat_precise_shot, feat_point_blank_shot, feat_manyshot,
feat_mounted_combat, feat_mounted_archery, feat_far_shot, feat_improved_bull_rush,
feat_weapon_focus_head, feat_weapon_specialization_head, feat_greater_weapon_focus_head, feat_greater_weapon_specialization,
"Melee Weapon Mastery", "Vexing Flanker", "Ranged Weapon Mastery", "Agile Shield Fighter", "Active Shield Defense",
"Shield Charge", "Shield Specialization", "Shield Ward"]

class_skills = (skill_alchemy, skill_climb, skill_craft, skill_handle_animal, skill_intimidate, skill_jump, skill_ride, skill_swim)

def GetHitDieType():
	return 10
	
def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 0
	
def IsWillSaveFavored():
	return 0
def GetSpellListType():
	return spell_list_type_none

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats


def IsAlignmentCompatible( alignment):
	return 1

def ObjMeetsPrereqs( obj ):
	return 1


# Levelup

def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl % 2 == 0 or newLvl == 1:
		return 1
	return 0

def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	for ft in bonus_feats:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	char_editor.set_bonus_feats(bonFeatInfo)
	return 
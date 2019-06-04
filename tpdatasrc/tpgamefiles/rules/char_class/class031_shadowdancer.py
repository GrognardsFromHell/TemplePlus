from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Shadowdancer"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_SHADOWDANCERS"

classEnum = stat_level_shadowdancer

###################################################


class_feats = {
1: (feat_armor_proficiency_light, feat_martial_weapon_proficiency_shortbow, feat_martial_weapon_proficiency_composite_shortbow, feat_martial_weapon_proficiency_rapier, feat_martial_weapon_proficiency_short_sword)
}

class_skills = (skill_alchemy, skill_balance, skill_bluff, skill_decipher_script, skill_diplomacy, skill_disguise, skill_escape_artist, skill_hide, skill_jump, skill_listen, skill_move_silently, skill_perform, skill_profession, skill_search, skill_pick_pocket, skill_spot, skill_tumble, skill_use_rope)

def IsEnabled():
	return 0

def GetHitDieType():
	return 8

def GetSkillPtsPerLevel():
	return 6
	
def GetBabProgression():
	return base_attack_bonus_semi_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 1

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
	return 0 # WIP
	if (obj.skill_ranks_get(skill_move_silently) < 8):
		return 0
	if (obj.skill_ranks_get(skill_hide) < 10):
		return 0
	if (obj.skill_ranks_get(skill_perform) < 5):
		return 0
	if (not obj.has_feat(feat_combat_reflexes)):
		return 0
	if (not obj.has_feat(feat_dodge)):
		return 0
	if (not obj.has_feat(feat_mobility)):
		return 0
	return 1
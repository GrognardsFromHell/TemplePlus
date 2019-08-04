# static requirements - specified in class_specs.tab
# skillpts / level - class_specs.tab
# feats - should be specified in dictionary
from toee import *
import char_class_utils


###################################################

def GetConditionName(): # used by API
	return "Barbarian"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_BARBARIANS"

classEnum = stat_level_barbarian

###################################################


class_feats = {
1: (feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all, feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_shield_proficiency, feat_barbarian_rage, feat_fast_movement),
2: (feat_uncanny_dodge,),
5: (feat_improved_uncanny_dodge,),
11: (feat_greater_rage,),
14: (feat_indomitable_will,),
17: (feat_tireless_rage,),
20: (feat_mighty_rage,)
}

class_skills = (skill_alchemy, skill_climb, skill_craft, skill_handle_animal, skill_intimidate, skill_jump, skill_listen, skill_ride, skill_wilderness_lore, skill_swim)

def GetHitDieType():
	return 12
	
def GetSkillPtsPerLevel():
	return 4
	
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
	if (alignment & ALIGNMENT_LAWFUL) != 0:
		return 0
	return 1

def ObjMeetsPrereqs( obj ):
	return 1
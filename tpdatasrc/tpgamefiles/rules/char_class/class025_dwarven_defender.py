from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Dwarven Defender"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_DWARVEN_DEFENDERS"

classEnum = stat_level_dwarven_defender

###################################################


class_feats = {
1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all),
2: (feat_uncanny_dodge,),
4: (feat_traps,),
6: (feat_improved_uncanny_dodge,)
}

class_skills = (skill_craft, skill_listen, skill_sense_motive, skill_spot)

def IsEnabled():
	return 1

def GetHitDieType():
	return 12

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_martial

def IsFortSaveFavored():
	return 1

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1
def GetSpellListType():
	return spell_list_type_none

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	if (alignment & ALIGNMENT_LAWFUL) == 0:
		return 0
	return 1


def ObjMeetsPrereqs( obj ):
	if obj.get_base_attack_bonus() < 7:
		return 0
	if obj.stat_base_get(stat_race) != race_dwarf:
		return 0
	if not obj.has_feat(feat_dodge):
		return 0
	# if (not obj.has_feat(feat_endurance) ): # endurance not implemented in ToEE
		# return 0
	if not obj.has_feat(feat_toughness):
		return 0
	return 1
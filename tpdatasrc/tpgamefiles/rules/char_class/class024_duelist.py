from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Duelist"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_DUELISTS"

classEnum = stat_level_duelist

###################################################


class_feats = {
1: ( feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all)
}

class_skills = (skill_balance, skill_bluff, skill_escape_artist, skill_jump, skill_listen, skill_perform, skill_sense_motive, skill_spot, skill_tumble)

def IsEnabled():
	return 1

def GetHitDieType():
	return 10

def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_type_martial

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

def IsAlignmentCompatible( alignment ):
	return 1


def ObjMeetsPrereqs( obj ):
	if obj.get_base_attack_bonus() < 6:
		return 0
	if obj.skill_ranks_get(skill_perform) < 3:
		return 0
	if obj.skill_ranks_get(skill_tumble) < 5:
		return 0
	if not obj.has_feat(feat_dodge):
		return 0
	if not obj.has_feat(feat_mobility):
		return 0
	if not obj.has_feat(feat_weapon_finesse_dagger): # this got changed to encompass all the weapon finesse feats
		return 0
	return 1
	
def IsSelectingFeatsOnLevelup( obj ):
	return 0
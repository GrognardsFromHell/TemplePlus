from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Horizon Walker"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_HORIZON_WALKERS"

classEnum = stat_level_horizon_walker

###################################################


class_feats = {
}



class_skills = (skill_balance, skill_climb, skill_diplomacy, skill_handle_animal, skill_hide, skill_knowledge_nature, skill_listen, skill_move_silently, skill_profession, skill_ride, skill_spot, skill_wilderness_lore)

def IsEnabled():
	return 0

def GetHitDieType():
	return 8

def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_martial

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
	return 0 # WIP
	if (obj.divine_spell_level_can_cast() < 7):
		return 0
	return 1
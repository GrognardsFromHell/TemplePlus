from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Thaumaturgist"

def GetSpellCasterConditionName():
	return "Thaumaturgist Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_THAUMATURGISTS"

classEnum = stat_level_thaumaturgist

###################################################

class_feats = {
}

class_skills = (skill_concentration, skill_craft, skill_diplomacy, skill_knowledge_religion, skill_profession, skill_sense_motive, skill_spellcraft)

def IsEnabled():
	return 0

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_non_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1
def GetSpellListType():
	return spell_list_type_any

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
	if (obj.divine_spell_level_can_cast() < 7): # in lieu of Lesser Planar Ally
		return 0
	if (not obj.has_feat(feat_spell_focus_conjuration)):
		return 0
	return 1
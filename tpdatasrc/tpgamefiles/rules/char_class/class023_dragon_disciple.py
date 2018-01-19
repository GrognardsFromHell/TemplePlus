from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Dragon Disciple"

def GetSpellCasterConditionName():
	return "Dragon Disciple Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_DRAGON_DISCIPLES"

classEnum = stat_level_dragon_disciple

###################################################


class_feats = {
}

class_skills = (skill_concentration, skill_craft, skill_diplomacy, skill_escape_artist, skill_gather_information, skill_knowledge_all, skill_listen, skill_profession, skill_search, skill_spellcraft, skill_spot)

def IsEnabled():
	return 0

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
	return spell_list_type_none # dragon disciples only advance bonus spells

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	return 1

def CanCastInnateArcane(obj): #todo: generalize
	if obj.stat_level_get(stat_level_bard)>0:
		return 1
	if obj.stat_level_get(stat_level_sorcerer) > 0:
		return 1
	return 0
	
def SpeaksDraconic( obj ):
	return 1 # languages not implemented in ToEE

def ObjMeetsPrereqs( obj ):
	return 0 # WIP
	# if (obj.skill_ranks_get(skill_knowledge_arcana) < 8): #knowledge skill not implemented in ToEE
		# return 0
	if obj.stat_level_get(stat_level) < 5: # a replacement for checking knowledge arcana
		return 0
	if (not obj.has_feat(feat_cleave) ):
		return 0
	if (not obj.has_feat(feat_power_attack) ):
		return 0
	if (not CanCastInnateArcane(obj)):
		return 0
	# if (not obj.has_feat(feat_improved_sunder) ): # sunder not yet implemented
		# return 0
	if (not obj.d20_query('Made contact with evil outsider')):
		return 0
	return 1
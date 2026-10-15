from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Dragon Disciple"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_DRAGON_DISCIPLES"

classEnum = stat_level_dragon_disciple

###################################################


class_feats = {
1: ("Dragon Disciple Natural Armor", "Dragon Disciple Bonus Spells", "Dragon Disciple Heritage"),
2: ("Dragon Disciple Claws and Bite", "Dragon Disciple Ability Boost"),
3: ("Dragon Disciple Breath Weapon",),
5: ("Dragon Disciple Blindsense",),
9: ("Dragon Disciple Wings",),
10: ("Dragon Disciple Dragon Apotheosis",),
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_escape_artist, skill_gather_information, skill_knowledge_all, skill_listen, skill_profession, skill_search, skill_spellcraft, skill_spot)

def IsEnabled():
	return 1

def GetHitDieType():
	return 12

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial # 3/4 (SRD)

def IsFortSaveFavored():
	return 1

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1
def GetSpellListType():
	return spell_list_type_none # dragon disciples don't advance casting; they get bonus spells per day (see dragon_disciple.py)

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	return 1

def CanCastInnateArcane(obj):
	# "able to cast arcane spells without preparation" (sorcerer, bard, and any Temple+ spontaneous arcane class)
	return obj.highest_spontaneous_arcane_class > 0

def SpeaksDraconic( obj ):
	return 1 # languages are not implemented in ToEE

def KnowledgeArcanaCheck( obj ):
	# Knowledge (arcana) 8 ranks
	return max(obj.skill_ranks_get(skill_knowledge_arcana), obj.skill_ranks_get(skill_knowledge_all)) >= 8

def ObjMeetsPrereqs( obj ):
	if not KnowledgeArcanaCheck(obj):
		return 0
	if not CanCastInnateArcane(obj):
		return 0
	return 1

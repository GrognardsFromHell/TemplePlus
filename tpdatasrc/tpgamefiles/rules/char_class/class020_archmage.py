from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Archmage"

def GetSpellCasterConditionName():
	return "Archmage Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ARCHMAGES"

classEnum = stat_level_archmage

###################################################


class_feats = {
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_knowledge_all, skill_profession, skill_search, skill_spellcraft)

def IsEnabled():
	return 0

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_non_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1

def GetSpellListType():
	return spell_list_type_arcane

def GetSpellSourceType():
	return spell_source_type_arcane

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	return 1


def CanCastArcaneLvl7(obj):
	# TODO: generalize (to support other arcane classes)
	if obj.stat_level_get(stat_level_sorcerer) >= 14:
		return 1
	if obj.stat_level_get(stat_level_wizard) >= 13:
		return 1

def HasSpellFocusInTwoSchool( obj ):
	sf1 = 0
	for p in range(feat_spell_focus_abjuration, feat_spell_focus_transmutation+1):
		if obj.has_feat(p):
			sf1 = p
			break
	if sf1 == 0:
		return 0
	
	sf2 = 0
	for p in range(feat_spell_focus_abjuration, feat_spell_focus_transmutation+1):
		if obj.has_feat(p) and p != sf1:
			sf2 = p
			break
	if sf2 == 0:
		return 0
	return 1
		
def ObjMeetsPrereqs( obj ):
	return 0 # WIP
	# skill ranks (only Disable Device since Escape Artist, Decipher Script and Knowledge Arcana aren't implemented in ToEE)
	if obj.skill_ranks_get(skill_spellcraft) < 15:
		return 0
	if (not obj.has_feat(feat_skill_focus_spellcraft)):
		return 0
	
	if (not CanCastArcaneLvl7(obj)):
		return 0
	if (not HasSpellFocusInTwoSchool(obj)):
		return 0
	return 1
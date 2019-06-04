from toee import *
import char_class_utils

###################################################

def GetConditionName(): # used by API
	return "Arcane Archer"

def GetSpellCasterConditionName():
	return "Arcane Archer Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ARCANE_ARCHERS"

classEnum = stat_level_arcane_archer

###################################################


class_feats = {
	1: (feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all,
	    feat_armor_proficiency_light, feat_armor_proficiency_medium,  feat_shield_proficiency, "Enhance Arrow"),
	2: ("Imbue Arrow",),
	4: ("Seeker Arrow",),
	6: ("Phase Arrow",),
	8: ("Hail of Arrows",),
	10: ("Arrow of Death",)
}


class_skills = (skill_alchemy, skill_craft, skill_hide, skill_listen, skill_move_silently, skill_ride, skill_spot, skill_wilderness_lore, skill_use_rope)

def IsEnabled():
	return 1

def GetHitDieType():
	return 8
	
def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_type_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 1
	
def IsWillSaveFavored():
	return 0

def GetSpellListType():
	return spell_list_type_none

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
	
def ObjMeetsPrereqs( obj ):
	objRace = obj.stat_level_get(stat_race)
	if not(objRace == race_elf or objRace == race_halfelf):
		return 0
	if obj.get_base_attack_bonus() < 6:
		return 0
	if not (obj.has_feat(feat_point_blank_shot)):
		return 0
	if not (obj.has_feat(feat_precise_shot)):
		return 0
	if not (obj.has_feat(feat_weapon_focus_longbow)) and not (obj.has_feat(feat_weapon_focus_shortbow)):
		return 0
	if obj.arcane_spell_level_can_cast() < 1:
		return 0
	return 1
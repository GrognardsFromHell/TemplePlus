from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Cleric"

# def GetSpellCasterConditionName():
	# return "Cleric Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_CLERICS"
	
classEnum = stat_level_cleric

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_domain_power)
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_heal, skill_knowledge_arcana, skill_knowledge_religion, skill_profession, skill_spellcraft)

# not including domain spells
spells_per_day = {
1:  (3, 1),
2:  (4, 2),
3:  (4, 2, 1),
4:  (5, 3, 2),
5:  (5, 3, 2, 1),
6:  (5, 3, 3, 2),
7:  (6, 4, 3, 2, 1),
8:  (6, 4, 3, 3, 2),
9:  (6, 4, 4, 3, 2, 1),
10: (6, 4, 4, 3, 3, 2),
11: (6, 5, 4, 4, 3, 2, 1),
12: (6, 5, 4, 4, 3, 3, 2),
13: (6, 5, 5, 4, 4, 3, 2, 1),
14: (6, 5, 5, 4, 4, 3, 3, 2),
15: (6, 5, 5, 5, 4, 4, 3, 2, 1),
16: (6, 5, 5, 5, 4, 4, 3, 3, 2),
17: (6, 5, 5, 5, 5, 4, 4, 3, 2, 1),
18: (6, 5, 5, 5, 5, 4, 4, 3, 3, 2),
19: (6, 5, 5, 5, 5, 5, 4, 4, 3, 3),
20: (6, 5, 5, 5, 5, 5, 4, 4, 4, 4)
#lvl 0  1  2  3  4  5  6  7  8  9
}

def GetHitDieType():
	return 8
	
def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 0
	
def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_clerical

def GetSpellSourceType():
	return spell_source_type_divine

def GetSpellReadyingType():
	return spell_readying_vancian

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
def GetCasterLevels():
	return caster_levels

def GetSpellDeterminingStat():
	return stat_wisdom

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	return 1

def ObjMeetsPrereqs( obj ):
	abScore = obj.stat_base_get(stat_wisdom)
	if abScore > 10:
		return 1
	return 0

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1

	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells)

	# Domain spells:
	domain_1 = obj.obj_get_int(obj_f_critter_domain_1)
	domain_2 = obj.obj_get_int(obj_f_critter_domain_2)
	domain_1_spells = char_editor.get_learnable_spells(obj, domain_1, maxSpellLvl, 1)
	domain_2_spells = char_editor.get_learnable_spells(obj, domain_2, maxSpellLvl, 1)
	char_editor.spell_known_add(domain_1_spells)
	char_editor.spell_known_add(domain_2_spells)
	return 0
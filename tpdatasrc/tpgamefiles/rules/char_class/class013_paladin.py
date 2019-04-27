from toee import *
import char_class_utils
import char_editor

###################################################

def GetConditionName(): # used by API
	return "Paladin"

def GetSpellCasterConditionName():
	return "Paladin Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_PALADINS"

classEnum = stat_level_paladin

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_smite_evil, feat_martial_weapon_proficiency_all, feat_detect_evil, feat_code_of_conduct),
2: (feat_lay_on_hands, feat_divine_grace),
3: (feat_aura_of_courage, feat_divine_health),
4: (feat_turn_undead,),
5: (feat_special_mount,),
6: (feat_remove_disease,)
}

class_skills = (skill_concentration, skill_craft, skill_diplomacy, skill_handle_animal, skill_heal, skill_knowledge_religion, skill_profession, skill_ride, skill_sense_motive)

spells_per_day = {
1:  (-1,),
2:  (-1,),
3:  (-1,),
4:  (-1,0),
5:  (-1,0),
6:  (-1,1),
7:  (-1,1),
8:  (-1,1, 0),
9:  (-1,1, 0),
10: (-1,1, 1),
11: (-1,1, 1, 0),
12: (-1,1, 1, 1),
13: (-1,1, 1, 1),
14: (-1,2, 1, 1, 0),
15: (-1,2, 1, 1, 1),
16: (-1,2, 2, 1, 1),
17: (-1,2, 2, 2, 1),
18: (-1,3, 2, 2, 1),
19: (-1,3, 3, 3, 2),
20: (-1,3, 3, 3, 3)
#lvl 0  1  2  3  4  5  6  7  8  9
}

def GetHitDieType():
	return 10
	
def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 0
	
def IsWillSaveFavored():
	return 0
	
#Spell casting
def GetSpellListType():
	return spell_list_type_paladin

def GetSpellSourceType():
	return spell_source_type_divine

def GetSpellReadyingType():
	return spell_readying_vancian

def GetSpellsPerDay():
	return spells_per_day

caster_levels = [ int(x / 2) if x >= 4 else 0 for x in range(1, 21) ]
def GetCasterLevels(classLvl):
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
	if ALIGNMENT_LAWFUL_GOOD != alignment:
		return 0
	return 1

def ObjMeetsPrereqs( obj ):
	if obj.d20_query(Q_IsFallenPaladin):
		return 0
	if obj.char_classes.count > 0:
		palLevel = obj.stat_level_get(stat_level_paladin)
		if palLevel > 0 and obj.char_classes[len(obj.char_classes) - 1] != stat_level_paladin: # has non-paladin level in the last taken class - bar from taking a Paladin level
			return 0
	return 1

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	if classLvlNew < 4: # late-starting caster
		return 0
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells)
	return 0
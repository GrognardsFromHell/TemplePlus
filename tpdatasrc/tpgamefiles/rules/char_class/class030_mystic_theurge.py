from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Mystic Theurge"

def GetSpellCasterConditionName():
	return "Mystic Theurge Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_MYSTIC_THEURGES"

classEnum = stat_level_mystic_theurge

###################################################

class_feats = {
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_decipher_script, skill_knowledge_arcana, skill_knowledge_religion, skill_profession, skill_sense_motive, skill_spellcraft)



def IsEnabled():
	return 1

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
	return spell_list_type_extender

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1



def ObjMeetsPrereqs( obj ):
	if obj.divine_spell_level_can_cast() < 2 or obj.arcane_spell_level_can_cast() < 2:
		return 0
	if obj.stat_level_get(stat_level) < 3: # in lieu of Knowledge ranks
		return 0
	return 1


# Levelup
def IsSelectingSpellsOnLevelup( obj , class_extended_1 = 0, class_extended_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestDivineClass(obj)
	if char_editor.is_selecting_spells(obj, class_extended_1):
		return 1
	if char_editor.is_selecting_spells(obj, class_extended_2):
		return 1
	return 0

def LevelupCheckSpells( obj , class_extended_1 = 0, class_extended_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestDivineClass(obj)
	if not char_editor.spells_check_complete(obj, class_extended_1):
		return 0
	if not char_editor.spells_check_complete(obj, class_extended_2):
		return 0
	return 1

def InitSpellSelection( obj , class_extended_1 = 0, class_extended_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestDivineClass(obj)
	char_editor.init_spell_selection(obj, class_extended_1)
	char_editor.init_spell_selection(obj, class_extended_2)
	
def LevelupSpellsFinalize( obj , class_extended_1 = 0, class_extended_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestDivineClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	char_editor.spells_finalize(obj, class_extended_2)
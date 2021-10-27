from toee import *
import char_class_utils

###################################################

def GetConditionName():
	return "Holy Scourge"

def GetSpellCasterConditionName():
	return "Holy Scourge Spellcasting"

def GetCategory():
	return "Complete Mange Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_None

def GetClassHelpTopic():
	return "TAG_HOLY_SCOURGE"

classEnum = stat_level_holy_scourge

###################################################


class_feats = {
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_knowledge_arcana, skill_knowledge_religion, skill_profession, skill_spellcraft)

def IsEnabled():
	return 1

def GetHitDieType():
	return 6

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

def IsAlignmentCompatible(alignment):
	if (alignment & ALIGNMENT_GOOD) == 0:
		return 0
	return 1

		
def ObjMeetsPrereqs( obj ):
	# skill ranks knowledge relegion 2 (no knowledge relegion in in ToEE)
	
	if obj.arcane_spell_level_can_cast() < 3:
		return 0
		
	#3 Evocation spells known
	count = 0
	known_spells = obj.spells_known
	for knSp in known_spells:
			spell_entry = tpdp.SpellEntry(knSp.spell_enum)
			if spell_entry.spell_school_enum == Evocation:
				count = count + 1

	if count < 3:
		return 0
	
	return 1
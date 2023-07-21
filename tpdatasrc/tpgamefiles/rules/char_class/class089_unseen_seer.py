from toee import *
import tpdp
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Unseen Seer"

def GetSpellCasterConditionName():
	return "Unseen Seer Spellcasting"

def GetCategory():
	return "Complete Mage Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_None

def GetClassHelpTopic():
	return "TAG_UNSEEN_SEER"

classEnum = stat_level_unseen_seer

###################################################

#Note:  Guraded mind at level 5 not implimented since there is no nondetection condition
class_feats = {
1: ("Unseen Seer Damage Bonus",),
2: (feat_silent_spell, "Unseen Seer Advanced Learning"),
3: ("Divination Spell Power",),
}

class_skills = (skill_alchemy, skill_bluff, skill_concentration, skill_decipher_script, skill_diplomacy, skill_disguise, skill_forgery, skill_gather_information, skill_hide, skill_knowledge_all, skill_listen, skill_move_silently, skill_profession, skill_search, skill_sense_motive, skill_spellcraft, skill_spot)

def IsEnabled():
	return 1

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 6
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_extender

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
	if obj.skill_ranks_get(skill_hide) < 8:
		return 0
	if obj.skill_ranks_get(skill_search) < 8:
		return 0
	if obj.skill_ranks_get(skill_spellcraft) < 4:
		return 0
	if obj.skill_ranks_get(skill_sense_motive) < 4:
		return 0
	if obj.skill_ranks_get(skill_spot) < 8:
		return 0
		
	#Can cast arcane spells
	if obj.arcane_spell_level_can_cast() < 1:
		return 0
		
	#Knows 2 divination spells
	divination_spells_known = 0
	known_spells = obj.spells_known
	for knSp in known_spells:
		if knSp.spell_level > 0:
			spell_entry = tpdp.SpellEntry(knSp.spell_enum)
			if spell_entry.spell_school_enum == Divination:
				divination_spells_known = divination_spells_known + 1
				if divination_spells_known > 1:
					break
					
	if divination_spells_known < 2:
		return 0
	return 1


# Levelup

def IsSelectingSpellsOnLevelup(obj, class_extended_1=0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	if char_editor.is_selecting_spells(obj, class_extended_1):
		return 1
	return 0


def LevelupCheckSpells(obj, class_extended_1=0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	if not char_editor.spells_check_complete(obj, class_extended_1):
		return 0
	return 1


def InitSpellSelection(obj, class_extended_1=0):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1 or class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.init_spell_selection(obj, class_extended_1)
	return 0


def LevelupSpellsFinalize(obj, class_extended_1=0):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1 or class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	
	if newLvl in [5,8]:
		obj.d20_send_signal("Add Advanced Learning Spell")
	return 0
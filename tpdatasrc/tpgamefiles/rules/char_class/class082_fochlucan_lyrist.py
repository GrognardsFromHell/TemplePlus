from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Fochlucan Lyrist"

def GetSpellCasterConditionName():
	return "Fochlucan Lyrist Spellcasting"

def GetCategory():
	return "Complete Adventurer Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_None

def GetClassHelpTopic():
	return "TAG_FOCHLUCAN_LYRISTS"

classEnum = stat_level_fochlucan_lyrist

###################################################
class_feats = {
1: ("Unbound", "Lyrist Bardic Music"),
}

class_skills = (skill_appraise, skill_bluff, skill_concentration, skill_craft, skill_decipher_script, skill_diplomacy, skill_disguise, skill_handle_animal, skill_knowledge_nature, skill_knowledge_arcana, skill_knowledge_religion, skill_listen, skill_move_silently, skill_heal, skill_hide, skill_perform, skill_pick_pocket, skill_use_magic_device, skill_sense_motive, skill_spellcraft)



def IsEnabled():
	return 1

def GetHitDieType():
	return 6

def GetSkillPtsPerLevel():
	return 6
	
def GetBabProgression():
	return base_attack_bonus_type_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 1

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
	if alignment == ALIGNMENT_NEUTRAL_GOOD or alignment == ALIGNMENT_NEUTRAL or alignment == ALIGNMENT_NEUTRAL_EVIL or alignment == ALIGNMENT_CHAOTIC_NEUTRAL:
		return 1
	return 0


def ObjMeetsPrereqs( obj ):
	if obj.skill_ranks_get(skill_perform) < 13:
		return 0
	if obj.skill_ranks_get(skill_diplomacy) < 7:
		return 0
	if obj.skill_ranks_get(skill_gather_information) < 7:
		return 0
	if obj.skill_ranks_get(skill_pick_pocket) < 7:
		return 0
	if not obj.has_feat(feat_evasion):
		return 0
	if not obj.has_feat(feat_bardic_knowledge):
		return 0
	if not obj.has_feat(feat_simple_weapon_proficiency_druid): #replaces requirement to speak Druidic
		return 0
	#Can cast first level divine and arcane spells
	if obj.divine_spell_level_can_cast() < 1 or obj.arcane_spell_level_can_cast() < 1:
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
	return 0
	
def LevelupSpellsFinalize( obj , class_extended_1 = 0, class_extended_2 = 0):
	if class_extended_1 <= 0 or class_extended_2 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
		class_extended_2 = char_class_utils.GetHighestDivineClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	char_editor.spells_finalize(obj, class_extended_2)
	return
	
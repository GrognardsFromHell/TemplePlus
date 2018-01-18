from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Arcane Trickster"

def GetSpellCasterConditionName():
	return "Arcane Trickster Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ARCANE_TRICKSTERS"

classEnum = stat_level_arcane_trickster

###################################################


class_feats = {
	2: (feat_sneak_attack,)
}

class_skills = (skill_appraise, skill_balance, skill_bluff, skill_climb, skill_concentration, skill_craft, skill_decipher_script, skill_diplomacy, skill_disable_device, skill_disguise, skill_escape_artist, skill_gather_information, skill_hide, skill_jump, skill_knowledge_all, skill_listen, skill_move_silently, skill_open_lock, skill_profession, skill_search, skill_sense_motive, skill_pick_pocket, skill_spellcraft, skill_spot, skill_swim, skill_tumble, skill_use_rope)


def IsEnabled():
	return 1

def GetHitDieType():
	return 4

def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_type_non_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 1

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
	# non-lawful
	if (alignment & ALIGNMENT_LAWFUL) != 0:
		return 0
	return 1

def HasSneak2d6( obj ):
	if obj.d20_query("Sneak Attack Dice") >= 2:
		return 1
	return 0

def CanCastArcaneLvl3(obj):
	# TODO: generalize (to support other arcane classes)
	if obj.stat_level_get(stat_level_sorcerer) >= 6:
		return 1
	if obj.stat_level_get(stat_level_wizard) >= 5:
		return 1
	if obj.stat_level_get(stat_level_bard) >= 7:
		return 1
	if obj.arcane_spell_level_can_cast() >= 3:
		return 1
	return 0

def ObjMeetsPrereqs( obj ):
	# skill ranks (only Disable Device, since Escape Artist, Decipher Script and Knowledge Arcana aren't implemented in ToEE)
	if obj.skill_ranks_get(skill_disable_device) < 7:
		return 0
	# magical hand - skipped (it's just cantrip anyway)
	
	if not HasSneak2d6(obj):
		return 0
	if not CanCastArcaneLvl3(obj):
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
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	return 0
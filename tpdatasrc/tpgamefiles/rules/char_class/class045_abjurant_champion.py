from toee import *
import char_class_utils
import char_editor
import tpdp
###################################################

def GetConditionName():
	return "Abjurant Champion"

def GetSpellCasterConditionName():
	return "Abjurant Champion Spellcasting"

def GetCategory():
	return "Complete Mage Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_None

def GetClassHelpTopic():
	return "TAG_ABJURANT_CHAMPION"

classEnum = stat_level_abjurant_champion

###################################################


class_feats = {
1: ("Abjurant Armor", "Extended Abjuration",),
2: ("Swift Abjuration", ),
4: ("Arcane Boost",),
5: ("Martial Arcanist",),
}

class_skills = (skill_climb, skill_concentration, skill_craft, skill_handle_animal, skill_intimidate, skill_jump, skill_knowledge_arcana, skill_ride, skill_spellcraft, skill_swim)


def IsEnabled():
	return 1

def GetHitDieType():
	return 10

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_extender

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible(alignment):
	return 1


def ObjMeetsPrereqs( obj ):
	
	#Maximum number of levels is 5
	classLvl = obj.stat_level_get(classEnum)
	if classLvl >= 5:
		return 0

	if obj.arcane_spell_level_can_cast() < 1:
		return 0
	if obj.get_base_attack_bonus() < 5:
		return 0
	if not obj.has_feat(feat_combat_casting):
		return 0
		
	#Can Cast 1st level or greater abjuration
	can_cast_abjuration = false
	known_spells = obj.spells_known
	for knSp in known_spells:
		if knSp.spell_level > 0:
			spell_entry = tpdp.SpellEntry(knSp.spell_enum)
			if spell_entry.spell_school_enum == Abjuration:
				can_cast_abjuration = true
				break
	
	#Beguilers should qualify, they get Undetectable Alignment at first level which is not in the game
	if obj.stat_level_get(stat_level_beguiler) > 0:
		can_cast_abjuration = true
		
	#Warmages should qualify at level 4, when they get Fire Trap which is not in the game
	if obj.stat_level_get(stat_level_warmage) > 3:
		can_cast_abjuration = true
			
	if not can_cast_abjuration:
		return 0
		
	#Check for martial weapon proficiency
	if obj.has_feat(feat_martial_weapon_proficiency_all):
		return 1
			
	has_martial_feat = false
	for i in range (feat_martial_weapon_proficiency_throwing_axe , feat_martial_weapon_proficiency_composite_longbow):
		if char_editor.has_feat(i):
			has_martial_feat = true
	return has_martial_feat

# Levelup

def IsSelectingFeatsOnLevelup( obj ):
	return 0

def LevelupGetBonusFeats( obj ):
	return

def IsSelectingSpellsOnLevelup( obj , class_extended_1 = 0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	if char_editor.is_selecting_spells(obj, class_extended_1):
		return 1
	return 0


def LevelupCheckSpells( obj , class_extended_1 = 0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	if not char_editor.spells_check_complete(obj, class_extended_1):
		return 0
	return 1
	
def InitSpellSelection( obj , class_extended_1 = 0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.init_spell_selection(obj, class_extended_1)
	return 0

def LevelupSpellsFinalize( obj , class_extended_1 = 0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	return 0
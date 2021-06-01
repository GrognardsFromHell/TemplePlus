from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Assassin"

# def GetSpellCasterConditionName():
# 	return "Assassin Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ASSASSINS"

classEnum = stat_level_assassin

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency_rogue, feat_sneak_attack, "Death Attack"),
2: (feat_uncanny_dodge, "Save Bonus against Poison"),
5: (feat_improved_uncanny_dodge,),
8: ("Hide in Plain Sight",),
}

class_skills = (skill_alchemy, skill_balance, skill_bluff, skill_climb, skill_craft, skill_decipher_script, skill_diplomacy, skill_disable_device, skill_disguise, skill_escape_artist, skill_forgery, skill_gather_information, skill_hide, skill_intimidate, skill_jump, skill_listen, skill_move_silently, skill_open_lock, skill_search, skill_sense_motive, skill_pick_pocket, skill_spot, skill_swim, skill_tumble, skill_use_magic_device, skill_use_rope)

# note:
# added grease to level 1 spells in place of the spells that are not implemented in ToEE
# added Blindness/Deafness and Ghoul Touch to level 2
# added slow and vampiric touch to level 3
spell_list = {
	1: (spell_critical_strike, spell_disguise_self, spell_detect_poison, spell_distract_assailant, spell_feather_fall, spell_ghost_sound, spell_grease, spell_insightfull_feint, spell_jump, spell_lightfoot, spell_obscuring_mist, spell_shock_and_awe, spell_sleep, spell_snipers_shot, spell_sticky_fingers, spell_true_strike),
	2: (spell_alter_self, spell_blindness_deafness, spell_cats_grace, spell_fell_the_greatest_foe, spell_fire_shuriken, spell_foxs_cunning, spell_ghoul_touch, spell_illusory_script, spell_invisibility, spell_invisibility_swift, spell_pass_without_trace, spell_phantom_foe, spell_spider_climb, spell_veil_of_shadow, spell_undetectable_alignment),
	3: (spell_deep_slumber, spell_deeper_darkness, spell_false_life, spell_find_the_gap, spell_magic_circle_against_good, spell_slow, spell_vampiric_touch, spell_misdirection, spell_nondetection, spell_wraithstrike),
	4: (spell_clairaudience_clairvoyance, spell_dimension_door, spell_freedom_of_movement, spell_glibness, spell_heart_ripper, spell_improved_invisibility, spell_locate_creature, spell_modify_memory, spell_poison)
}

spells_per_day = {
1:  (-1,0),
2:  (-1,1),
3:  (-1,2, 0),
4:  (-1,3, 1),
5:  (-1,3, 2, 0),
6:  (-1,3, 3, 1),
7:  (-1,3, 3, 2, 0),
8:  (-1,3, 3, 3, 1),
9:  (-1,3, 3, 3, 2),
10: (-1,3, 3, 3, 3)
}

spells_known = {
1:  (0,2),
2:  (0,3),
3:  (0,3, 2),
4:  (0,4, 3),
5:  (0,4, 3, 2),
6:  (0,4, 4, 3),
7:  (0,4, 4, 3, 2),
8:  (0,4, 4, 4, 3),
9:  (0,4, 4, 4, 3),
10: (0,4, 4, 4, 4)
}

def IsEnabled():
	return 1

def GetHitDieType():
	return 6

def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 1

def IsWillSaveFavored():
	return 0

# Spell casting
def GetSpellListType():
	return spell_list_type_special

def GetSpellSourceType():
	return spell_source_type_arcane

def GetSpellReadyingType():
	return spell_readying_innate
	
def HasArmoredArcaneCasterFeature():
	return 1

def GetSpellList():
	return spell_list

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 11)
def GetCasterLevels():
	return caster_levels

def GetSpellDeterminingStat():
	return stat_intelligence

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	if not (alignment & ALIGNMENT_EVIL):
		return 0
	return 1


def ObjMeetsPrereqs( obj ):
	#return 0 # WIP
	# skill ranks (only Disable Device since Escape Artist, Decipher Script and Knowledge Arcana aren't implemented in ToEE)
	if obj.skill_ranks_get(skill_hide) < 8:
		return 0
	if obj.skill_ranks_get(skill_move_silently) < 8:
		return 0
	#if not obj.d20_query('Killed to join Assassins'):
	#	return 0
	return 1

# Levelup callbacks
def IsSelectingSpellsOnLevelup(obj):
	return 1


def InitSpellSelection(obj, classLvlNew = -1, classLvlIncrement = 1):
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level(obj, classEnum,
	                                              classLvlNew)  # this regards spell list extension by stuff like Mystic Theurge (no need to use spellListLvl as below)

	# Available Spells
	spAvail = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	# add spell level labels
	for p in range(0, maxSpellLvl + 1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()
	char_editor.append_available_spells(spAvail)

	# Spell Slots

	spellListLvl = obj.stat_level_get(stat_spell_list_level,
	                                  classEnum) + 1  # the effective level for getting the number of spells known
	spEnums = char_editor.get_known_class_spells(obj, classEnum)  # get all spells known for this class

	for spellLvl in range(1, maxSpellLvl + 1):
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0 + spellLvl, 0, classEnum))  # add label
		# add spell slots
		newSpellsKnownCount = char_class_utils.GetSpellsKnownAddedCount(spells_known, spellListLvl, spellLvl)
		for q in range(0, newSpellsKnownCount):
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0 + spellLvl, 3, classEnum))

	isReplacing = 0
	if spellListLvl >= 6 and ((spellListLvl - 6) % 2) == 0:  # spell replacement
		isReplacing = 1
	if char_editor.get_class_code() != classEnum:  # grant this benefit only for strict levelup (also to prevent some headache...)
		isReplacing = 0

	if isReplacing == 0:
		spEnums.sort()
		char_editor.append_spell_enums(spEnums)
		return 0

	# mark as replaceable
	for p in range(0, len(spEnums)):
		spEnum = spEnums[p].spell_enum
		if spell_vacant <= spEnum <= spell_label_level_9:
			continue
		if spell_new_slot_lvl_0 <= spEnum <= spell_new_slot_lvl_9:
			continue
		if char_editor.get_spell_level(spEnum, classEnum) <= maxSpellLvl - 2:
			spEnums[p].spell_status = 1

	spEnums.sort()
	char_editor.append_spell_enums(spEnums)
	return 0


def LevelupCheckSpells(obj):
	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			return 0
	return 1


def LevelupSpellsFinalize(obj, classLvlNew = -1 ):
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums)  # internally takes care of duplicates and the labels/vacant slots
	return
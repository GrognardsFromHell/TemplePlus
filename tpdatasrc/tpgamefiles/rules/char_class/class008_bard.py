from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Bard"

# def GetSpellCasterConditionName():
	# return "Bard Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_BARDS"

classEnum = stat_level_bard

###################################################

class_feats = {

1: (feat_simple_weapon_proficiency_bard, feat_shield_proficiency,
 feat_bardic_music, feat_bardic_knowledge )
 #feat_martial_weapon_proficiency_composite_longbow,
 #feat_martial_weapon_proficiency_longbow, feat_martial_weapon_proficiency_longsword,
 #feat_martial_weapon_proficiency_rapier, feat_martial_weapon_proficiency_sap,
 #feat_martial_weapon_proficiency_composite_shortbow,
 #feat_martial_weapon_proficiency_short_sword,feat_martial_weapon_proficiency_shortbow )
}


class_skills = (skill_appraise, skill_balance, skill_bluff, skill_climb, skill_concentration, skill_craft, skill_decipher_script, skill_diplomacy, skill_decipher_script, skill_diplomacy, skill_disguise, skill_escape_artist, skill_gather_information, skill_hide, skill_jump, skill_knowledge_all, skill_listen, skill_move_silently, skill_perform, skill_profession, skill_sense_motive, skill_pick_pocket, skill_spellcraft, skill_swim, skill_tumble, skill_use_magic_device)


spells_per_day = {
1:  (2,),
2:  (3, 0),
3:  (3, 1),
4:  (3, 2, 0),
5:  (3, 3, 1),
6:  (3, 3, 2),
7:  (3, 3, 2, 0),
8:  (3, 3, 3, 1),
9:  (3, 3, 3, 2),
10: (3, 3, 3, 2, 0),
11: (3, 3, 3, 3, 1),
12: (3, 3, 3, 3, 2),
13: (3, 3, 3, 3, 2, 0),
14: (4, 3, 3, 3, 3, 1),
15: (4, 4, 3, 3, 3, 2),
16: (4, 4, 4, 3, 3, 2, 0),
17: (4, 4, 4, 4, 3, 3, 1),
18: (4, 4, 4, 4, 4, 3, 2),
19: (4, 4, 4, 4, 4, 4, 3),
20: (4, 4, 4, 4, 4, 4, 4)
#lvl 0  1  2  3  4  5  6  7  8  9
}

spells_known = {
1:  (4,),
2:  (5, 2),
3:  (6, 3),
4:  (6, 3, 2),
5:  (6, 4, 3),
6:  (6, 4, 3),
7:  (6, 4, 4, 2),
8:  (6, 4, 4, 3),
9:  (6, 4, 4, 3),
10: (6, 4, 4, 4, 2),
11: (6, 4, 4, 4, 3),
12: (6, 4, 4, 4, 3),
13: (6, 4, 4, 4, 4, 2),
14: (6, 4, 4, 4, 4, 3),
15: (6, 4, 4, 4, 4, 3),
16: (6, 5, 4, 4, 4, 4, 2),
17: (6, 5, 5, 4, 4, 4, 3),
18: (6, 5, 5, 5, 4, 4, 3),
19: (6, 5, 5, 5, 5, 4, 4),
20: (6, 5, 5, 5, 5, 5, 4)
#lvl 0  1  2  3  4  5  6  7  8  9
}

def GetHitDieType():
	return 6

def GetSkillPtsPerLevel():
	return 6

def GetBabProgression():
	return base_attack_bonus_type_semi_martial

def IsFortSaveFavored():
	return 0

def IsRefSaveFavored():
	return 1

def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_bardic

def GetSpellSourceType():
	return spell_source_type_arcane

def GetSpellReadyingType():
	return spell_readying_innate

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
def GetCasterLevels(classLvl):
	return caster_levels

def GetSpellDeterminingStat():
	return stat_charisma

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	if (alignment & ALIGNMENT_LAWFUL) != 0:
		return 0
	return 1

def ObjMeetsPrereqs( obj ):
	abScore = obj.stat_base_get(stat_charisma)
	if abScore > 10:
		return 1
	return 0

## Levelup callbacks

def IsSelectingSpellsOnLevelup( obj ):
	return 1

def InitSpellSelection( obj, classLvlNew = -1, classLvlIncrement = 1):
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew ) # this regards spell list extension by stuff like Mystic Theurge (no need to use spellListLvl as below)
	
	# Available Spells
	spAvail = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	# add spell level labels
	for p in range(0,maxSpellLvl+1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()
	char_editor.append_available_spells(spAvail)
	
	# Spell Slots
	if classLvlNew == 1: #newly taken class
		spEnums = []
		# 4 cantrips
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0, 0, classEnum)) # add "Level 0" label
		for p in range(0,4):
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0, 3, classEnum))
		char_editor.append_spell_enums(spEnums)
		return 0
	
	# Incrementing class level
	spellListLvl = obj.stat_level_get(stat_spell_list_level, classEnum) + classLvlIncrement # the effective level for getting the number of spells known
	spEnums = char_editor.get_known_class_spells(obj, classEnum) # get all spells known for this class
	
	for spellLvl in range(0, maxSpellLvl+1):
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0 + spellLvl, 0, classEnum))  # add label
		# add
		newSpellsKnownCount = char_class_utils.GetSpellsKnownAddedCount( spells_known , spellListLvl, spellLvl)
		for q in range(0, newSpellsKnownCount):
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0 + spellLvl, 3, classEnum))
	
	isReplacing = 0
	if spellListLvl >= 5 and ((spellListLvl - 5) % 3) == 0: # spell replacement
		isReplacing = 1
	if char_editor.get_class_code() !=  classEnum: #grant this benefit only for strict levelup (also to prevent some headache...)
		isReplacing = 0
	
	if isReplacing == 0:
		spEnums.sort()
		char_editor.append_spell_enums(spEnums)
		return 0
	
	# mark as replaceable
	for p in range(0,len(spEnums)):
		spEnum = spEnums[p].spell_enum
		if spell_vacant <= spEnum <= spell_label_level_9:
			continue
		if spell_new_slot_lvl_0 <= spEnum <= spell_new_slot_lvl_9:
			continue
		if char_editor.get_spell_level(spEnum, classEnum) <= maxSpellLvl-2:
			spEnums[p].spell_status = 1
	
	spEnums.sort()
	char_editor.append_spell_enums(spEnums)
	return 0

def LevelupCheckSpells( obj):
	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		spClass = spInfo.get_casting_class()
		if spClass != stat_level_bard:
			continue
		if spInfo.spell_enum == spell_vacant:
			return 0
	return 1

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots
	return
from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Sorcerer"

# def GetSpellCasterConditionName():
	# return "Sorcerer Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_SORCERERS"

classEnum = stat_level_sorcerer

###################################################

class_feats = {

1: (feat_simple_weapon_proficiency, feat_call_familiar)
}

class_skills = (skill_alchemy, skill_bluff, skill_concentration, skill_craft, skill_knowledge_arcana, skill_profession, skill_spellcraft)

spells_per_day = {
1:  (5, 3),
2:  (6, 4),
3:  (6, 5),
4:  (6, 6, 3),
5:  (6, 6, 4),
6:  (6, 6, 5, 3),
7:  (6, 6, 6, 4),
8:  (6, 6, 6, 5, 3),
9:  (6, 6, 6, 6, 4),
10: (6, 6, 6, 6, 5, 3),
11: (6, 6, 6, 6, 6, 4),
12: (6, 6, 6, 6, 6, 5, 3),
13: (6, 6, 6, 6, 6, 6, 4),
14: (6, 6, 6, 6, 6, 6, 5, 3),
15: (6, 6, 6, 6, 6, 6, 6, 4),
16: (6, 6, 6, 6, 6, 6, 6, 5, 3),
17: (6, 6, 6, 6, 6, 6, 6, 6, 4),
18: (6, 6, 6, 6, 6, 6, 6, 6, 5, 3),
19: (6, 6, 6, 6, 6, 6, 6, 6, 6, 4),
20: (6, 6, 6, 6, 6, 6, 6, 6, 6, 6)
#lvl 0  1  2  3  4  5  6  7  8  9
}

spells_known = {
1:  (4, 2),
2:  (5, 2),
3:  (5, 3),
4:  (6, 3, 1),
5:  (6, 4, 2),
6:  (7, 4, 2, 1),
7:  (7, 5, 3, 2),
8:  (8, 5, 3, 2, 1),
9:  (8, 5, 4, 3, 2),
10: (9, 5, 4, 3, 2, 1),
11: (9, 5, 5, 4, 3, 2),
12: (9, 5, 5, 4, 3, 2, 1),
13: (9, 5, 5, 4, 4, 3, 2),
14: (9, 5, 5, 4, 4, 3, 2, 1),
15: (9, 5, 5, 4, 4, 4, 3, 2),
16: (9, 5, 5, 4, 4, 4, 3, 2, 1),
17: (9, 5, 5, 4, 4, 4, 3, 3, 2),
18: (9, 5, 5, 4, 4, 4, 3, 3, 2, 1),
19: (9, 5, 5, 4, 4, 4, 3, 3, 3, 2),
20: (9, 5, 5, 4, 4, 4, 3, 3, 3, 3)
#lvl 0  1  2  3  4  5  6  7  8  9
}

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

# Spell casting
def GetSpellListType():
	return spell_list_type_arcane

def GetSpellSourceType():
	return spell_source_type_arcane

def GetSpellReadyingType():
	return spell_readying_innate

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
def GetCasterLevels():
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
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew ) # this regards spell list extension by stuff like Mystic Theurge
	
	# Available Spells
	spAvail = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	# add spell level labels
	for p in range(0,maxSpellLvl+1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()
	char_editor.append_available_spells(spAvail)
	
	# newly taken class
	if classLvlNew == 1:
		spEnums = []
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0, 0, classEnum)) # add "Level 0" label
		for p in range(0,4): # 4 cantrips
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0, 3, classEnum))
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_1, 0, classEnum)) # add "Level 1" label
		for p in range(0,2): # 2 level 1 spells
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_1, 3, classEnum))
		char_editor.append_spell_enums(spEnums)
		return 0
	
	# Incrementing class level
	spellListLvl = obj.stat_level_get(stat_spell_list_level, classEnum) + classLvlIncrement # the effective level for getting the number of spells known
	spEnums = char_editor.get_known_class_spells(obj, classEnum) # get all spells known for this class
	for spellLvl in range(0, maxSpellLvl+1):
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0 + spellLvl, 0, classEnum))  # add label
		# add spells
		newSpellsKnownCount = char_class_utils.GetSpellsKnownAddedCount( spells_known , spellListLvl, spellLvl)
		print "new num spells for spell level " + str(spellLvl) + ": " + str(newSpellsKnownCount)
		for q in range(0, newSpellsKnownCount):
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0 + spellLvl, 3, classEnum))
	
	isReplacing = 0
	if spellListLvl >= 4 and (spellListLvl % 2) == 0: # spell replacement
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
			spEnums[p].spell_status = 1 # marked as replaceable
	
	spEnums.sort()
	char_editor.append_spell_enums(spEnums)
	return 0

def LevelupCheckSpells( obj ):
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	
	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			if maxSpellLvl >= 4 and spInfo.spell_level == 0: # in case the cantrips are causing problems
				continue
			return 0
	return 1

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots
	return
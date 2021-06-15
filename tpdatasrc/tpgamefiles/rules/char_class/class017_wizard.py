from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Wizard"
	
# def GetSpellCasterConditionName():
	# return "Wizard Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_WIZARDS"

classEnum = stat_level_wizard

###################################################

class_feats = {

1: (feat_simple_weapon_proficiency_wizard, feat_scribe_scroll, feat_call_familiar)
}


bonus_feats = [feat_brew_potion, feat_craft_magic_arms_and_armor, feat_craft_rod, feat_craft_staff, feat_craft_wand, feat_craft_wondrous_item, 
feat_empower_spell, feat_enlarge_spell, feat_extend_spell, feat_forge_ring, feat_heighten_spell, feat_maximize_spell, feat_quicken_spell, 
feat_scribe_scroll, feat_silent_spell, feat_still_spell, feat_widen_spell
]

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_decipher_script, skill_knowledge_nature, skill_knowledge_all, skill_profession, skill_spellcraft)

spells_per_day = {
1:  (3, 1),
2:  (4, 2),
3:  (4, 2, 1),
4:  (4, 3, 2),
5:  (4, 3, 2, 1),
6:  (4, 3, 3, 2),
7:  (4, 4, 3, 2, 1),
8:  (4, 4, 3, 3, 2),
9:  (4, 4, 4, 3, 2, 1),
10: (4, 4, 4, 3, 3, 2),
11: (4, 4, 4, 4, 3, 2, 1),
12: (4, 4, 4, 4, 3, 3, 2),
13: (4, 4, 4, 4, 4, 3, 2, 1),
14: (4, 4, 4, 4, 4, 3, 3, 2),
15: (4, 4, 4, 4, 4, 4, 3, 2, 1),
16: (4, 4, 4, 4, 4, 4, 3, 3, 2),
17: (4, 4, 4, 4, 4, 4, 4, 3, 2, 1),
18: (4, 4, 4, 4, 4, 4, 4, 3, 3, 2),
19: (4, 4, 4, 4, 4, 4, 4, 4, 3, 3),
20: (4, 4, 4, 4, 4, 4, 4, 4, 4, 4)
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
	return spell_readying_vancian

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
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
	return 1
	
def ObjMeetsPrereqs( obj ):
	abScore = obj.stat_base_get(stat_intelligence)
	if abScore > 10:
		return 1
	return 0

## Levelup callbacks

def IsSelectingFeaturesOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1:
		return 1
	return 0

def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl % 5 == 0:
		return 1
	return 0

def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	for ft in bonus_feats:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	char_editor.set_bonus_feats(bonFeatInfo)
	return
	
	
def IsSelectingSpellsOnLevelup( obj ):
	# if char_editor.get_class_code() !=  classEnum:# This is strictly a Wizard benefit (in case it's being accessed externally from sthg like Mystic Theurge / Archmage)
	# 	return 0
	return 1

def InitSpellSelection( obj, classLvlNew = -1, classLvlIncrement = 1):

	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew ) # this regards spell list extension by stuff like Mystic Theurge
	
	# Available Spells
	spAvail = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	# cull cantrips
	p = 0
	while p < len(spAvail):
		if spAvail[p].spell_level == 0:
			del(spAvail[p])
		else:
			p = p+1
	# add spell level labels
	for p in range(1,maxSpellLvl+1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()
	char_editor.append_available_spells(spAvail)
	
	# Spell slots
	spEnums = []
	vacant_slot = char_editor.KnownSpellInfo(spell_vacant, 3, classEnum) # sets it to spell level -1
	if classLvlNew == 1: # newly taken class
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_1, 0, classEnum))
		for p in range(0,2):
			spEnums.append(vacant_slot)
		intScore = obj.stat_level_get(stat_intelligence)
		intMod = (intScore - 10) / 2
		for p in range(0, intMod):
			spEnums.append(vacant_slot)
	else: # add 2 new spell slots
		for p in range(0,2):
			spEnums.append(vacant_slot)
	char_editor.append_spell_enums(spEnums)
	
	return 0

def LevelupCheckSpells( obj):
	spell_enums = char_editor.get_spell_enums()
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	if classLvlNew == 2:
		sp_lvl_1_count = 0
		spKnown = char_editor.get_known_class_spells(obj, classEnum)
		for spInfo in spKnown:
			if spInfo.spell_level == 1:
				sp_lvl_1_count += 1
		if sp_lvl_1_count > 20: # for scroll puffing maniacs
			return 1
	
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			return 0
	return 1


def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots
	
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	if classLvlNew > 1:
		return 0
	# for new wizards, add all cantrips
	cantrips = char_editor.get_learnable_spells(obj, classEnum, 0)
	char_editor.spell_known_add(cantrips)
	return 0
from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Favored Soul"

# def GetSpellCasterConditionName():
	# return "Cleric Spellcasting"

def GetCategory():
	return "Complete Divine"

def GetClassDefinitionFlags():
	return CDF_BaseClass

def GetClassHelpTopic():
	return "TAG_FAVORED_SOULS"
	
classEnum = stat_level_favored_soul

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_domain_power),
3: ('Deity\'s Weapon Focus',),
5: ('Energy Resistance (Favored Soul)',),
12: ('Deity\'s Weapon Specialization',),
20: ('Damage Reduction (Favored Soul)',)
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_heal, skill_knowledge_arcana, skill_profession, skill_sense_motive, skill_spellcraft)

# not including domain spells
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
1:  (4, 3),
2:  (5, 3),
3:  (5, 4),
4:  (6, 4, 3),
5:  (6, 5, 3),
6:  (7, 5, 4, 3),
7:  (7, 6, 4, 3),
8:  (8, 6, 5, 4, 3),
9:  (8, 6, 5, 4, 3),
10: (9, 6, 6, 5, 4, 3),
11: (9, 6, 6, 5, 4, 3),
12: (9, 6, 6, 6, 5, 4, 3),
13: (9, 6, 6, 6, 5, 4, 3),
14: (9, 6, 6, 6, 6, 5, 4, 3),
15: (9, 6, 6, 6, 6, 5, 4, 3),
16: (9, 6, 6, 6, 6, 6, 5, 4, 3),
17: (9, 6, 6, 6, 6, 6, 5, 4, 3),
18: (9, 6, 6, 6, 6, 6, 6, 5, 4, 3),
19: (9, 6, 6, 6, 6, 6, 6, 5, 4, 3),
20: (9, 6, 6, 6, 6, 6, 6, 6, 5, 4)
#lvl 0  1  2  3  4  5  6  7  8  9
}

def IsEnabled():
	return 1

def GetHitDieType():
	return 8
	
def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 1
	
def IsWillSaveFavored():
	return 1

# Spell casting
def GetSpellListType():
	return spell_list_type_clerical

def GetSpellSourceType():
	return spell_source_type_divine

def GetSpellReadyingType():
	return spell_readying_innate

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
def GetCasterLevels():
	return caster_levels

def GetSpellDeterminingStat():
	return stat_charisma

def GetSpellDcStat():
	return stat_wisdom

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

def GetDeityClass():
	return stat_level_cleric

## Levelup


# Spells
def IsSelectingSpellsOnLevelup( obj ):
	return 1


def InitSpellSelection(obj, classLvlNew = -1, classLvlIncrement = 1):
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level(obj, classEnum,
	                                              classLvlNew)  # this regards spell list extension by stuff like Mystic Theurge

	# Available Spells
	spAvail = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	# add spell level labels
	for p in range(0, maxSpellLvl + 1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()
	char_editor.append_available_spells(spAvail)

	# newly taken class
	if classLvlNew == 1:
		spEnums = []
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0, 0, classEnum))  # add "Level 0" label
		for p in range(0, 4):  # 4 cantrips
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0, 3, classEnum))
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_1, 0, classEnum))  # add "Level 1" label
		for p in range(0, 3):  # 2 level 1 spells
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_1, 3, classEnum))
		char_editor.append_spell_enums(spEnums)
		return 0

	# Incrementing class level
	spellListLvl = obj.stat_level_get(stat_spell_list_level,
	                                  classEnum) + classLvlIncrement  # the effective level for getting the number of spells known
	spEnums = char_editor.get_known_class_spells(obj, classEnum)  # get all spells known for this class
	for spellLvl in range(0, maxSpellLvl + 1):
		spEnums.append(char_editor.KnownSpellInfo(spell_label_level_0 + spellLvl, 0, classEnum))  # add label
		# add spells
		newSpellsKnownCount = char_class_utils.GetSpellsKnownAddedCount(spells_known, spellListLvl, spellLvl)
		
		#Do not add more spells than can be selected (otherwise lockup happens)
		spellList = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
		spellAvailableCount = 0
		for spell in spellList:
			if spellLvl == spell.spell_level:
				if not obj.is_spell_known(spell.spell_enum):
					spellAvailableCount = spellAvailableCount + 1
		newSpellsKnownCount = min(newSpellsKnownCount, spellAvailableCount)
		
		print str(newSpellsKnownCount)
		for q in range(0, newSpellsKnownCount):
			spEnums.append(char_editor.KnownSpellInfo(spell_new_slot_lvl_0 + spellLvl, 3, classEnum))

	isReplacing = 0
	if spellListLvl >= 4 and (spellListLvl % 2) == 0:  # spell replacement
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
			spEnums[p].spell_status = 1  # marked as replaceable

	spEnums.sort()
	char_editor.append_spell_enums(spEnums)
	return 0


def LevelupCheckSpells(obj):
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level(obj, classEnum, classLvlNew)

	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			if maxSpellLvl >= 4 and spInfo.spell_level == 0:  # in case the cantrips are causing problems
				continue
			return 0
	return 1

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	spEnums = char_editor.get_spell_enums()
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots

	# add weapon focus / specialization
	newLvl = obj.stat_level_get(classEnum) + 1
	if newLvl != 3 and newLvl != 12 and classLvlNew != 1:
		return

	omg = obj.obj_get_int(obj_f_critter_deity)
	deity_weapon = game.get_deity_favored_weapon(omg)
	#print "deity weap " + str(deity_weapon)
	if newLvl == 3:
		weap_foc_feat = feat_weapon_focus_gauntlet + deity_weapon
		obj.feat_add(weap_foc_feat )
		print "added " + str(weap_foc_feat)
	if newLvl == 12:
		weap_sp_feat = feat_weapon_specialization_gauntlet + deity_weapon
		obj.feat_add(weap_sp_feat)
		print "added " + str(weap_sp_feat)
	if classLvlNew == 1:
		weap_prof_feat = game.get_feat_for_weapon_type(deity_weapon)
		if weap_prof_feat != feat_simple_weapon_proficiency:
			obj.feat_add(weap_prof_feat)
			print "added " + str(weap_prof_feat)
	return


# Feats

def IsSelectingFeatsOnLevelup(obj):
	newLvl = obj.stat_level_get(classEnum) + 1
	if newLvl == 5 or newLvl == 10 or newLvl == 15:
		return 1
	if newLvl != 3 and newLvl != 12:
		return 0

	omg = obj.obj_get_int(obj_f_critter_deity)
	deity_weapon = game.get_deity_favored_weapon(omg)
	if newLvl == 3:
		weap_foc_feat = feat_weapon_focus_gauntlet + deity_weapon
		if obj.has_feat(weap_foc_feat):
			return 1
	#if newLvl == 12:
	#	weap_sp_feat = feat_weapon_specialization_gauntlet + deity_weapon
	#	if obj.has_feat(weap_sp_feat):
	#		return 1
	return 0


def LevelupGetBonusFeats(obj):
	newLvl = obj.stat_level_get(classEnum) + 1
	bonus_feats = []
	if newLvl == 5 or newLvl == 10 or newLvl == 15:
		bonus_feats = [
			'Favored Soul Cold Resistance',
			'Favored Soul Fire Resistance',
			'Favored Soul Acid Resistance',
			'Favored Soul Electricity Resistance',
			'Favored Soul Sonic Resistance'
		]
	elif newLvl == 3 or newLvl == 12:
		omg = obj.obj_get_int(obj_f_critter_deity)
		deity_weapon = game.get_deity_favored_weapon(omg)
		if newLvl == 3:
			weap_foc_feat = feat_weapon_focus_gauntlet + deity_weapon
			if obj.has_feat(weap_foc_feat):
				bonus_feats = [feat_weapon_focus_head,]
		#elif newLvl == 12:
		#	weap_sp_feat = feat_weapon_specialization_gauntlet + deity_weapon
		#	if obj.has_feat(weap_sp_feat):
		#		bonus_feats = [feat_weapon_specialization_head, ]


	bonFeatInfo = []
	for ft in bonus_feats:
		featInfo = char_editor.FeatInfo(ft)
		featInfo.feat_status_flags |= 4 # always pickable
		bonFeatInfo.append(featInfo)
	char_editor.set_bonus_feats(bonFeatInfo)
	return
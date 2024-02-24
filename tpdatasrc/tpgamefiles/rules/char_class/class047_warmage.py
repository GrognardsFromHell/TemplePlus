from toee import *
import char_class_utils
import char_editor
import tpdp
import functools

# Warmage:  Complete Arcane, p. 10

###################################################

def GetConditionName(): # used by API
	return "Warmage"

def GetCategory():
	return "Complete Arcane"

def GetClassDefinitionFlags():
	return CDF_BaseClass

def GetClassHelpTopic():
	return "TAG_WARMAGES"
	
classEnum = stat_level_warmage

###################################################

#Note:  The shield proficency feat is used here but warmages should not get proficency with heavy shields (only light).  This may be worth updating at some point. 
class_feats = {
1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency, "Light Shield Proficiency", "Warmage Armored Mage", "Warmage Edge"),
8: (feat_armor_proficiency_medium,),
}

class_skills = (skill_alchemy, skill_concentration, skill_intimidate, skill_spellcraft)

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
20: (6, 6, 6, 6, 6, 6, 6, 6, 6, 5)
#lvl 0  1  2  3  4  5  6  7  8  9
}

# There are non core spells that are not listed since they are not in constants.py.  If they are added to the game they should be added here.
# Warmages are granted all the spells on their list as soon as they get atleast one spell per day of that level.  Additional spells are added
# to this list with advanced learning.
spell_list = {
	0: (spell_acid_splash, spell_disrupt_undead, spell_light, spell_ray_of_frost),
	1: (spell_accuracy, spell_burning_hands, spell_hail_of_stone, spell_lesser_orb_of_acid, spell_lesser_orb_of_cold, spell_lesser_orb_of_electricity, spell_lesser_orb_of_fire, spell_lesser_orb_of_sound, spell_chill_touch, spell_magic_missile, spell_shocking_grasp, spell_true_strike),
	2: (spell_blades_of_fire, spell_continual_flame, spell_fire_trap, spell_fireburst, spell_flaming_sphere, spell_ice_knife, spell_melfs_acid_arrow, spell_pyrotechnics, spell_shatter, spell_scorching_ray, spell_whirling_blade),
	3: (spell_fire_shield, spell_fireball, spell_flame_arrow, spell_gust_of_wind, spell_ice_storm, spell_lightning_bolt, spell_poison, spell_ring_of_blades, spell_sleet_storm, spell_stinking_cloud),
	4: (spell_blast_of_flame, spell_contagion, spell_evards_black_tentacles, spell_orb_of_acid, spell_orb_of_cold, spell_orb_of_electricity, spell_orb_of_fire, spell_orb_of_sound, spell_phantasmal_killer, spell_shout, spell_wall_of_fire),
	5: (spell_cloudkill, spell_cone_of_cold, spell_flame_strike, spell_fire_shield_mass, spell_greater_fireburst, spell_prismatic_ray),
	6: (spell_blade_barrier, spell_chain_lightning, spell_circle_of_death, spell_disintegrate, spell_fire_seeds, spell_otilukes_freezing_sphere, spell_tensers_transformation),
	7: (spell_delayed_blast_fireball, spell_earthquake, spell_finger_of_death, spell_fire_storm, spell_mordenkainens_sword, spell_prismatic_spray, spell_sunbeam, ),
	8: (spell_horrid_wilting, spell_incendiary_cloud, spell_polar_ray, spell_prismatic_wall, spell_sunburst),
	9: (spell_elemental_swarm, spell_implosion, spell_meteor_swarm, spell_prismatic_sphere, spell_wail_of_the_banshee, spell_weird)
}

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

# Spell casting
def GetSpellListType():
	return spell_list_type_special

def GetSpellSourceType():
	return spell_source_type_arcane

def GetSpellReadyingType():
	return spell_readying_innate
	
def HasArmoredArcaneCasterFeature():
	return 1
	
def HasAdvancedLearning():
	return 1
	
def GetAdvancedLearningSpellLevel(spellEnum):
	spEntry = tpdp.SpellEntry(spellEnum)
	spellLevel = spEntry.level_for_spell_class(tpdp.class_enum_to_casting_class(stat_level_wizard))
	if spEntry.spell_school_enum != Evocation:
		spellLevel = spellLevel + 1
	return spellLevel
		
def GetSpellList():
	return spell_list

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 21)
def GetCasterLevels():
	return caster_levels

def GetSpellDeterminingStat():
	return stat_charisma

def GetSpellDcStat():
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

def GetDeityClass():
	return stat_level_sorcerer

#Now also supports electic learning from the players handbook 2 p. 67 
def IsAdvancedLearningSpell(obj, max_level, spell):
	spEntry = tpdp.SpellEntry(spell.spell_enum)
	
	#Don't add spells that are already known to the list
	if obj.is_spell_known(spell.spell_enum):
		return False
	
	#First get rid of everything in the warmage spell list
	for level, spell_list_level in spell_list.items():
		if spell.spell_enum in spell_list_level:
			return False
		
	#For the max level, it must be evocation, all spells lower then that are electic learning spells
	if (spell.spell_level > max_level) or (spell.spell_level == max_level and spEntry.spell_school_enum != Evocation):
		return False
	
	return True

def GetAdvancedLearningList(obj, maxSpellLevel):
	#Add wizard spells and remove all that are not evocation
	
	spAdvancedLearningList = char_editor.get_learnable_spells(obj, stat_level_wizard, maxSpellLevel)
	spAdvancedLearningList = filter(functools.partial(IsAdvancedLearningSpell, obj, maxSpellLevel), spAdvancedLearningList)
	
	#Fix the class enum and the level for non evocation spells
	for idx in range(0, len(spAdvancedLearningList)):
		spAdvancedLearningList[idx].set_casting_class(classEnum)
		spEntry = tpdp.SpellEntry(spAdvancedLearningList[idx].spell_enum)
		if spEntry.spell_school_enum != Evocation:
			spAdvancedLearningList[idx].spell_level = spAdvancedLearningList[idx].spell_level + 1
	
	return spAdvancedLearningList
	
def IsSelectingSpellsOnLevelup(obj):
	if char_editor.get_class_code() !=  classEnum: # This is strictly a warmage benefit (in case it's being accessed externally from something like Mystic Theurge / Archmage)
		return 0
	
	classLvl = obj.stat_level_get(classEnum)
	classLvlNew = classLvl + 1
	
	#levels 3, 6, 11, and 16 get advanced learning otherwise there is nothing to select
	if classLvlNew in [3, 6, 11, 16]:
		maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
		AdvancedLearningList = GetAdvancedLearningList(obj, maxSpellLvl)
		if AdvancedLearningList:
			return 1
	return 0
	
def InitSpellSelection(obj, classLvlNew = -1, classLvlIncrement = 1):
	if char_editor.get_class_code() !=  classEnum: # This is strictly a warmage benefit (in case it's being accessed externally from something like Mystic Theurge / Archmage)
		return 0

	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1
	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	
	spAvail = GetAdvancedLearningList(obj, maxSpellLvl)
	
	# Add the spell level labels
	for p in range(0,maxSpellLvl+1):
		spAvail.append(char_editor.KnownSpellInfo(spell_label_level_0 + p, 0, classEnum))
	spAvail.sort()

	char_editor.append_available_spells(spAvail)
	
	# Add a single spell slot
	vacant_slot = char_editor.KnownSpellInfo(spell_vacant, 3, classEnum)
	spEnums = [vacant_slot]
	char_editor.append_spell_enums(spEnums)
	return 0
	
def LevelupCheckSpells(obj):
	spell_enums = char_editor.get_spell_enums()
	for spInfo in spell_enums:
		if spInfo.spell_enum == spell_vacant:
			return 0
	return 1
	
def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	#Add the normal spells
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1

	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells) # internally takes care of duplicates and the labels/vacant slots	
	
	#Add Anything from advanced learning
	spEnums = char_editor.get_spell_enums() #The correct level is lost here...
	char_editor.spell_known_add(spEnums) # internally takes care of duplicates and the labels/vacant slots	
	return 0
	
def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if (newLvl == 7) or (newLvl == 10) or (newLvl == 15) or (newLvl == 20):
		return 1
	return 0
	
def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	feat = ""
	newLvl = obj.stat_level_get( classEnum ) + 1
	#Find the normal feat for each level
	if newLvl == 7:
		feat = "Sudden Empower"
	elif newLvl == 10:
		feat = "Sudden Enlarge"
	elif newLvl == 15:
		feat = "Sudden Widen"
	elif newLvl == 20:
		feat = "Sudden Maximize"
	else:
		return #No bonus feat this level
	
	#The only option will be the normal feat if the chracter does not have it
	if not char_editor.has_feat(feat):
		featInfo = char_editor.FeatInfo(feat)
		featInfo.feat_status_flags |= FEAT_INFO_DISREGARD_PREREQS # always pickable
		bonFeatInfo.append(featInfo)
	else:
		#Any metamagic feat can be selected if the character does not have the normal feat
		bonus_feats = tpdp.get_metamagic_feats()
		
		#If the character already has the feat, he can select any metamagic feat
		for feat in bonus_feats:
			bonFeatInfo.append(char_editor.FeatInfo(feat))
	
	char_editor.set_bonus_feats(bonFeatInfo)
	return


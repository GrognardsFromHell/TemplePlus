from toee import *
import char_class_utils
import char_editor
import tpdp

# Warmage:  Complete Arcane, p. 10

###################################################

def GetConditionName(): # used by API
	return "Warmage"

def GetCategory():
	return "Complete Arcane"

def GetClassDefinitionFlags():
	return CDF_BaseClass

def GetClassHelpTopic():
	return "TAG_WARMAGE"
	
classEnum = stat_level_warmage

###################################################

#Note:  The shield proficency feat is used here but warmages should not get proficency with heavy shields (only light).  This may be worth updating at some point. 
class_feats = {
1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency, feat_shield_proficiency, "Warmage Armored Mage", "Warmage Edge"),
8: (feat_armor_proficiency_medium,),
}

class_skills = (skill_concentration, skill_intimidate, skill_spellcraft)

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
# Warmages are granted all the spells on their list as soon as they get atleast one spell per day of that level.
# Advanced learning offers warmages additional evocation spells on the sorcerer/wizard list.  Currently this just adds wind wall for a third level 
# spell.  If more spells are added, this may need to be reworked.
spell_list = {
	0: (spell_acid_splash, spell_disrupt_undead, spell_light, spell_ray_of_frost),
	1: (spell_burning_hands, spell_chill_touch, spell_magic_missile, spell_shocking_grasp, spell_true_strike),
	2: (spell_continual_flame, spell_fire_trap, spell_flaming_sphere, spell_melfs_acid_arrow, spell_pyrotechnics, spell_shatter, spell_scorching_ray),
	3: (spell_fire_shield, spell_fireball, spell_flame_arrow, spell_gust_of_wind, spell_ice_storm, spell_lightning_bolt, spell_poison, spell_sleet_storm, spell_stinking_cloud, spell_wind_wall),
	4: (spell_contagion, spell_evards_black_tentacles, spell_phantasmal_killer, spell_shout, spell_wall_of_fire),
	5: (spell_cloudkill, spell_cone_of_cold, spell_flame_strike),
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
	
def GetSpellList():
	return spell_list

def GetSpellsPerDay():
	return spells_per_day

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
	
def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1

	maxSpellLvl = char_editor.get_max_spell_level( obj, classEnum, classLvlNew )
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells)
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
		bonFeatInfo.append(char_editor.FeatInfo(feat))
	else:
		#Any metamagic feat can be selected if the character does not have the normal feat
		bonus_feats = tpdp.get_metamagic_feats()
		
		#If the character already has the feat, he can select any metamagic feat
		for feat in bonus_feats:
			bonFeatInfo.append(char_editor.FeatInfo(feat))
	
	char_editor.set_bonus_feats(bonFeatInfo)
	return


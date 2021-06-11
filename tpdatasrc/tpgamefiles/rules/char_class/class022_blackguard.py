from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Blackguard"

def GetSpellCasterConditionName():
	return "Blackguard Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_BLACKGUARDS"

classEnum = stat_level_blackguard

###################################################


class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy,
    feat_shield_proficiency, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all, "Detect Good"),
2: ("Smite Good", "Dark Blessing"),
3: ("Aura of Despair", feat_rebuke_undead),
4: (feat_sneak_attack,)
}



class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_handle_animal, skill_heal, skill_hide, skill_intimidate, skill_knowledge_religion, skill_profession, skill_ride)


spell_list = {
	1: (spell_blessed_aim, spell_faith_healing, spell_cause_fear, spell_cure_light_wounds, spell_doom, spell_inflict_light_wounds, spell_magic_weapon, spell_strategic_charge, spell_summon_monster_i, spell_summon_undead_i),
	2: (spell_bulls_strength, spell_cure_moderate_wounds, spell_curse_of_ill_fortune, spell_darkness, spell_death_knell, spell_demonhide, spell_eagles_splendor, spell_hand_of_divinity, spell_mass_inflict_moderate_wounds, spell_shatter, spell_summon_monster_ii, spell_summon_undead_ii, spell_veil_of_shadow, spell_wave_of_grief),
	3: (spell_contagion, spell_cure_serious_wounds, spell_deeper_darkness, spell_inflict_serious_wounds, spell_protection_from_elements, spell_summon_monster_iii, spell_summon_undead_iii, spell_unholy_storm, spell_weapon_of_the_deity),
	4: (spell_cure_critical_wounds, spell_freedom_of_movement, spell_inflict_critical_wounds, spell_poison, spell_summon_monster_iv)
}

spells_per_day = {
1:  (-1,0),
2:  (-1,1),
3:  (-1,1, 0),
4:  (-1,1, 1),
5:  (-1,1, 1, 0),
6:  (-1,1, 1, 1),
7:  (-1,2, 1, 1, 0),
8:  (-1,2, 1, 1, 1),
9:  (-1,2, 2, 1, 1),
10: (-1,2, 2, 2, 1)
}

def IsEnabled():
	return 1

def GetHitDieType():
	return 10

def GetSkillPtsPerLevel():
	return 2
	
def GetBabProgression():
	return base_attack_bonus_type_martial

def IsFortSaveFavored():
	return 1

def IsRefSaveFavored():
	return 0

def IsWillSaveFavored():
	return 0

# Spell Casting
def GetSpellListType():
	return spell_list_type_special

def GetSpellSourceType():
	return spell_source_type_divine

def GetSpellReadyingType():
	return spell_readying_vancian

def GetSpellList():
	return spell_list

def GetSpellsPerDay():
	return spells_per_day

caster_levels = range(1, 11)
def GetCasterLevels():
	return caster_levels

def GetSpellDeterminingStat():
	return stat_wisdom

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	#if not (alignment & ALIGNMENT_EVIL):
	#	return 0
	## removed above to allow fallen paladins
	return 1


def ObjMeetsPrereqs( obj ):
	if obj.get_base_attack_bonus() < 6:
		return 0
	algn = obj.stat_level_get(stat_alignment)
	if not (algn & ALIGNMENT_EVIL) and (not game.is_lax_rules()):
		palLvl = obj.stat_level_get(stat_level_paladin)
		#is_fallen = obj.d20_query(Q_IsFallenPaladin) # changed this to just let Paladins fall if they pick the class
		#if not is_fallen:
		#	return 0
		if palLvl <= 0:
			return 0
	if obj.skill_ranks_get(skill_hide) < 5:
		return 0
	# if (obj.skill_ranks_get(skill_knowledge_religion) < 2): #knowledge skill not implemented in ToEE
		# return 0
	if not obj.has_feat(feat_cleave):
		return 0
	if not obj.has_feat(feat_power_attack):
		return 0
	# if (not obj.has_feat(feat_improved_sunder) ): # sunder not yet implemented
		# return 0
	#if not obj.d20_query("Made contact with evil outsider"):
	#	return 0
	return 1

def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	classLvl = obj.stat_level_get(classEnum)
	if classLvlNew <= 0:
		classLvlNew = classLvl + 1

	maxSpellLvl = char_editor.get_max_spell_level(obj, classEnum, classLvlNew)
	class_spells = char_editor.get_learnable_spells(obj, classEnum, maxSpellLvl)
	char_editor.spell_known_add(class_spells)

	# for paladins - change to evil
	algn = obj.obj_get_int(obj_f_critter_alignment)
	if algn & ALIGNMENT_GOOD:
		algn = algn ^ ALIGNMENT_GOOD

	algn = algn | ALIGNMENT_EVIL
	obj.obj_set_int(obj_f_critter_alignment, algn)

	pal_lvl = obj.stat_level_get(stat_level_paladin)
	if pal_lvl >= 1:
		obj.feat_add("Smite Good")
	if pal_lvl >= 5:
		obj.feat_add(feat_sneak_attack)
	return 0
from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
	return "Eldritch Knight"

def GetSpellCasterConditionName():
	return "Eldritch Knight Spellcasting"

def GetCategory():
	return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
	return CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ELDRITCH_KNIGHTS"

classEnum = stat_level_eldritch_knight

###################################################


class_feats = {
}

bonus_feats = [feat_blind_fight, feat_power_attack, feat_cleave, feat_great_cleave, feat_combat_expertise, 
feat_improved_disarm, feat_improved_feint, feat_improved_trip, feat_whirlwind_attack,
feat_combat_reflexes, feat_dodge, feat_mobility, feat_spring_attack, 
feat_martial_weapon_proficiency_head, feat_improved_critical_head, feat_improved_initiative,
feat_improved_unarmed_strike, feat_deflect_arrows, feat_improved_grapple, 
feat_improved_overrun, feat_improved_shield_bash , feat_improved_two_weapon_fighting,
feat_weapon_finesse_head, feat_greater_two_weapon_fighting, feat_sunder,
feat_two_weapon_defense, feat_two_weapon_fighting,  feat_improved_precise_shot,
feat_trample, feat_stunning_fist, feat_spring_attack, feat_spirited_charge,
feat_snatch_arrows, feat_shot_on_the_run, feat_ride_by_attack, feat_rapid_reload,
feat_rapid_shot, feat_quick_draw, feat_precise_shot, feat_point_blank_shot, feat_manyshot,
feat_mounted_combat, feat_mounted_archery, feat_far_shot, feat_improved_bull_rush,
feat_weapon_focus_head, feat_weapon_specialization_head, feat_greater_weapon_focus_head, feat_greater_weapon_specialization, feat_blind_fight, feat_combat_expertise]


class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_decipher_script, skill_jump, skill_knowledge_arcana, skill_ride, skill_sense_motive, skill_spellcraft, skill_swim)


def IsEnabled():
	return 1

def GetHitDieType():
	return 6

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

# Spell casting
def GetSpellListType():
	return spell_list_type_extender

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats
	
def IsAlignmentCompatible( alignment):
	return 1


def ObjMeetsPrereqs( obj ):
	if obj.arcane_spell_level_can_cast() < 3:
		return 0
	if not obj.has_feat(feat_martial_weapon_proficiency_all):
		return 0
	return 1

# Levelup

def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1:
		return 1
	return 0

def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	for ft in bonus_feats:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	char_editor.set_bonus_feats(bonFeatInfo)
	return

def IsSelectingSpellsOnLevelup( obj , class_extended_1 = 0):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1:
		return 0
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
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl == 1:
		return 0
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.init_spell_selection(obj, class_extended_1)
	return 0

def LevelupSpellsFinalize( obj , class_extended_1 = 0):
	if class_extended_1 <= 0:
		class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
	char_editor.spells_finalize(obj, class_extended_1)
	return 0
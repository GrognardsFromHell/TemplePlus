from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
	return "Rogue"

def GetCategory():
	return "Core 3.5 Ed Classes"

def GetClassDefinitionFlags():
	return CDF_BaseClass | CDF_CoreClass

def GetClassHelpTopic():
	return "TAG_ROGUES"

classEnum = stat_level_rogue

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency_rogue, feat_sneak_attack, feat_traps),
2: (feat_evasion,),
4: (feat_uncanny_dodge,),
8: (feat_improved_uncanny_dodge,),
}

bonus_feats = [feat_defensive_roll, feat_improved_evasion, feat_crippling_strike, feat_opportunist, feat_skill_mastery, feat_slippery_mind]

class_skills = (skill_alchemy, skill_appraise, skill_balance, skill_bluff, skill_climb, skill_craft, skill_decipher_script, skill_diplomacy, skill_disable_device, skill_disguise, skill_escape_artist, skill_forgery, skill_gather_information, skill_hide, skill_intimidate, skill_jump, skill_listen, skill_move_silently, skill_open_lock, skill_perform, skill_profession, skill_search, skill_sense_motive, skill_pick_pocket, skill_spot, skill_swim, skill_tumble, skill_use_magic_device, skill_use_rope)


def GetHitDieType():
	return 6
	
def GetSkillPtsPerLevel():
	return 8
	
def GetBabProgression():
	return base_attack_bonus_type_semi_martial
	
def IsFortSaveFavored():
	return 0
	
def IsRefSaveFavored():
	return 1
	
def IsWillSaveFavored():
	return 0
def GetSpellListType():
	return spell_list_type_none

def IsClassSkill(skillEnum):
	return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
	return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
	return class_feats

def IsAlignmentCompatible( alignment):
	return 1

def ObjMeetsPrereqs( obj ):
	return 1

# Levelup

def IsSelectingFeatsOnLevelup( obj ):
	newLvl = obj.stat_level_get( classEnum ) + 1
	if newLvl >= 10 and ((newLvl-10) % 3) == 0:
		return 1
	return 0

def LevelupGetBonusFeats( obj ):
	bonFeatInfo = []
	for ft in bonus_feats:
		bonFeatInfo.append(char_editor.FeatInfo(ft))
	char_editor.set_bonus_feats(bonFeatInfo)
	return 
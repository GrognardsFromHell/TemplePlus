from toee import *
import char_class_utils
import char_editor

#Complete Warrior, p. 11

###################################################

def GetConditionName(): # used by API
	return "Swashbuckler"

def GetCategory():
	return "Complete Warrior"

def GetClassDefinitionFlags():
	return CDF_BaseClass

def GetClassHelpTopic():
	return "TAG_SWASHBUCKLERS"

classEnum = stat_level_swashbuckler

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all, feat_weapon_finesse_dagger,),
2: ("Swashbuckler Grace",),
3: ("Swashbuckler Insightful Strike",),
5: ("Swashbuckler Dodge",),
7: ("Swashbuckler Acrobatic Charge",),
8: ("Swashbuckler Improved Flanking",),
11: ("Swashbuckler Lucky",),
13: ("Swashbuckler Acrobatic Skill Mastery",),
14: ("Swashbuckler Weakening Critical",),
17: (feat_slippery_mind,),
19: ("Swashbuckler Wounding Critical",),
}

class_skills = (skill_balance, skill_bluff, skill_climb, skill_diplomacy, skill_escape_artist, skill_sense_motive, skill_tumble, skill_use_rope)

def IsEnabled():
	return 1

def GetDeityClass():
	return stat_level_fighter

def GetHitDieType():
	return 10
	
def GetSkillPtsPerLevel():
	return 4
	
def GetBabProgression():
	return base_attack_bonus_type_martial
	
def IsFortSaveFavored():
	return 1
	
def IsRefSaveFavored():
	return 0
	
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
	
def IsSelectingFeatsOnLevelup( obj ):
	return 0
	
def LevelupGetBonusFeats( obj ):
	return
	
def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
	return 0

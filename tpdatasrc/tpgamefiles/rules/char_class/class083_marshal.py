from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName(): # used by API
    return "Marshal"

def GetCategory():
    return "Miniatures Handbook Base Classes"

def GetClassDefinitionFlags():
    return CDF_BaseClass

def GetClassHelpTopic():
    return "TAG_MARSHALS"

classEnum = stat_level_marshal

###################################################

class_feats = {

1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, 
feat_martial_weapon_proficiency_all, feat_skill_focus_diplomacy, "Marshal Minor Auras"),
2: ("Marshal Major Auras",),
4: ("Marshal Adrenaline Boost",)
}

class_skills = (skill_bluff, skill_diplomacy, skill_handle_animal, skill_intimidate, skill_knowledge_all, skill_listen, skill_perform, skill_ride, skill_sense_motive, skill_spot, skill_wilderness_lore, skill_swim)

def IsEnabled():
    return 1

def GetHitDieType():
    return 8
    
def GetSkillPtsPerLevel():
    return 4
    
def GetBabProgression():
    return base_attack_bonus_type_semi_martial
    
def IsFortSaveFavored():
    return 1
    
def IsRefSaveFavored():
    return 0
    
def IsWillSaveFavored():
    return 1

def GetSpellListType():
    return spell_list_type_none

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible(alignment):
    return 1

def ObjMeetsPrereqs(obj):
    return 1

def GetDeityClass():
    return stat_level_fighter

 ## Levelup
def LevelupSpellsFinalize( obj, classLvlNew = -1 ):
    return 0
from toee import *
import char_class_utils
import char_editor

###################################################

def GetConditionName():
    return "Fist of the Forest"

def GetSpellCasterConditionName():
    return "Fist of the Forest Spellcasting"

def GetCategory():
    return "Complete Champion Prestige Classes"

def GetClassDefinitionFlags():
    return CDF_None

def GetClassHelpTopic():
    return "TAG_FIST_OF_THE_FOREST"

classEnum = stat_level_fist_of_the_forest

###################################################


class_feats = {
1: ("Fist of the Forest AC Bonus", "Fist of the Forest Fast Movement", "Fist of the Forest Feral Trance", "Fist of the Forest Primal Living", "Fist of the Forest Unarmed Damage"),
2: (feat_uncanny_dodge, "Fist of the Forest Untamed Strike"),
3: ("Fist of the Forest Scent",)
}

class_skills = (skill_balance, skill_climb, skill_handle_animal, skill_intimidate, skill_jump, skill_listen, skill_move_silently, skill_sense_motive, skill_spot, skill_wilderness_lore, skill_swim)

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

# Prereqs
# Omitted: Must gain approval as a fist of the forest by the leaders of a band of Guardians of the Green.

def ObjMeetsPrereqs(obj):
    if char_editor.stat_level_get(stat_attack_bonus ) < 4:
        return 0
    #Handle Animal is not in the game
    #elif char_editor.skill_ranks_get(skill_handle_animal) < 4:
    #    return 0
    elif char_editor.skill_ranks_get(skill_wilderness_lore) < 4:
        return 0
    elif not char_editor.has_feat(feat_great_fortitude):
        return 0
    elif not char_editor.has_feat(feat_improved_unarmed_strike):
        return 0
    elif not char_editor.has_feat(feat_power_attack):
        return 0
    return 1


# Levelup
def IsSelectingFeatsOnLevelup(obj):
    return 0

def LevelupGetBonusFeats(obj):
    return


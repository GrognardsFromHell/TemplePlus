from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
    return "Stormlord"

def GetSpellCasterConditionName():
    return "Stormlord Spellcasting"

def GetCategory():
    return "Complete Divine Prestige Classes"

def GetClassDefinitionFlags():
    return CDF_None

def GetClassHelpTopic():
    return "TAG_STROMLORD"

classEnum = stat_level_stormlord

###################################################


class_feats = {
1: ("Stormlord Enhanced Javelins", "Stormlord Resistance to Electricity",),
2: ("Stormlord Shock Weapon",),
3: ("Stormlord Storm Walk",),
5: ("Stormlord Thundering Weapon",),
6: ("Stormlord Storm Ride",),
9: ("Stormlord Immunity to Electricity",),
10: ("Stormlord Storm of Elemental Fury",)
}

class_skills = (skill_concentration, skill_disguise, skill_gather_information, skill_intimidate, skill_knowledge_nature, skill_knowledge_religion, skill_swim, skill_wilderness_lore)


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
    return 0

def IsWillSaveFavored():
    return 1

# Spell casting
def GetSpellListType():
    return spell_list_type_extender

def GetSpellSourceType():
    return spell_source_type_divine

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible( alignment):
    # Alignment needs to be CE, CN or NE;
    allowedAlignments = [ALIGNMENT_CHAOTIC_EVIL, ALIGNMENT_CHAOTIC_NEUTRAL, ALIGNMENT_NEUTRAL_EVIL]
    if alignment in allowedAlignments:
        return 1
    else:
        return 0

def ObjMeetsPrereqs( obj ):
    requiredFeats = [feat_weapon_focus_halfspear, feat_weapon_focus_shortspear, feat_weapon_focus_longspear, feat_weapon_focus_javelin]
    objFeats = obj.feats
    if obj.divine_spell_level_can_cast() < 3:
        return 0
    elif obj.stat_level_get(stat_save_fortitude) < 4:
        return 0
#    elif not obj.has_feat(feat_endurance): #not in the game currently
#        return 0
    elif not obj.has_feat(feat_great_fortitude):
        return 0
    elif not any(feat in objFeats for feat in requiredFeats):
        return 0
    else:
        return 1


# Levelup

def IsSelectingSpellsOnLevelup(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestDivineClass(obj)
    if char_editor.is_selecting_spells(obj, class_extended_1):
        return 1
    return 0


def LevelupCheckSpells(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestDivineClass(obj)
    if not char_editor.spells_check_complete(obj, class_extended_1):
        return 0
    return 1


def InitSpellSelection(obj, class_extended_1=0):
    newLvl = obj.stat_level_get( classEnum ) + 1
    if newLvl == 1 or class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestDivineClass(obj)
    char_editor.init_spell_selection(obj, class_extended_1)
    return 0


def LevelupSpellsFinalize(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestDivineClass(obj)
    char_editor.spells_finalize(obj, class_extended_1)
    return 0
from toee import *
import char_class_utils
import char_editor
###################################################

def GetConditionName():
    return "Battle Howler of Gruumsh"

def GetSpellCasterConditionName():
    return "Battle Howler of Gruumsh Spellcasting"

def GetCategory():
    return "Dragon Magazine Prestige Class" #Dragon Magazine 311

def GetClassDefinitionFlags():
    return CDF_None

def GetClassHelpTopic():
    return "TAG_BATTLE_HOWLER_OF_GRUUMSH"

classEnum = stat_level_battle_howler_of_gruumsh

###################################################


class_feats = {
1: (feat_armor_proficiency_light, feat_armor_proficiency_medium, feat_armor_proficiency_heavy, feat_shield_proficiency, feat_simple_weapon_proficiency, feat_martial_weapon_proficiency_all, feat_bardic_music),
2: (feat_barbarian_rage),
3: ("Battle Howler of Gruumsh War Cry",),
4: ("Howling Cry",)
}

class_skills = (skill_appraise, skill_balance, skill_climb, skill_concentration, skill_craft, skill_handle_animal, skill_intimidate, skill_jump, skill_knowledge_all,
skill_listen, skill_perform, skill_ride, skill_search, skill_sense_motive, skill_spellcraft, skill_spot, skill_wilderness_lore, skill_swim)


def IsEnabled():
    return 1

def GetHitDieType():
    return 8

def GetSkillPtsPerLevel():
    return 4
    
def GetBabProgression():
    return base_attack_bonus_type_martial

def IsFortSaveFavored():
    return 2

def IsRefSaveFavored():
    return 0

def IsWillSaveFavored():
    return 0

# Spell casting
def GetSpellListType():
    return spell_list_type_extender

def GetSpellSourceType():
    return spell_source_type_arcane

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible(alignment):
    if alignment & ALIGNMENT_CHAOTIC:
        return 1
    return 0

def ObjMeetsPrereqs(obj):
    #Alignment: Any chaotic.
    #Skills: Knowledge (religion) 2 ranks, Perform (any) 8 ranks.
    #Feats: Cleave, Power Attack.
    #Deity: Must worship Gruumsh before all other gods.
    if char_editor.skill_ranks_get(skill_perform) < 8:
        return 0
    elif not char_editor.has_feat(feat_power_attack):
        return 0
    elif not char_editor.has_feat(feat_cleave):
        return 0
    elif not obj.get_deity() == DEITY_GRUUMSH:
        return 0
    return 1


# Levelup
def IsSelectingSpellsOnLevelup(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = stat_level_bard
    if char_editor.is_selecting_spells(obj, class_extended_1):
        return 1
    return 0


def LevelupCheckSpells(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = stat_level_bard
    if not char_editor.spells_check_complete(obj, class_extended_1):
        return 0
    return 1


def InitSpellSelection(obj, class_extended_1=0):
    newLvl = obj.stat_level_get( classEnum ) + 1
    if newLvl == 1 or class_extended_1 <= 0:
        class_extended_1 = stat_level_bard
    char_editor.init_spell_selection(obj, class_extended_1)
    return 0


def LevelupSpellsFinalize(obj, class_extended_1=0):
    if class_extended_1 <= 0:
        class_extended_1 = stat_level_bard
    char_editor.spells_finalize(obj, class_extended_1)
    return 0

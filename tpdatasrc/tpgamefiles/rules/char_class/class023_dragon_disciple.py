from toee import *
import char_class_utils
import char_editor

###################################################

def GetConditionName():
    return "Dragon Disciple"

def GetSpellCasterConditionName():
    return "Dragon Disciple Spellcasting"

def GetCategory():
    return "Core 3.5 Ed Prestige Classes"

def GetClassDefinitionFlags():
    return CDF_CoreClass

def GetClassHelpTopic():
    return "TAG_DRAGON_DISCIPLES"

classEnum = stat_level_dragon_disciple

###################################################


class_feats = {
1: ("Dragon Disciple Heritage", "Dragon Disciple Natural Armor",),
2: ("Dragon Disciple Claws and Bite",),
3: ("Dragon Disciple Breath Weapon",),
5: ("Dragon Disciple Blindsense",),
10: ("Dragon Disciple Dragon Apotheosis",)
}

class_skills = (skill_alchemy, skill_concentration, skill_craft, skill_diplomacy, skill_escape_artist, skill_gather_information, skill_knowledge_all, skill_listen, skill_profession, skill_search, skill_spellcraft, skill_spot)

def IsEnabled():
    return 1

def GetHitDieType():
    return 12

def GetSkillPtsPerLevel():
    return 2
    
def GetBabProgression():
    return base_attack_bonus_type_martial

def IsFortSaveFavored():
    return 1

def IsRefSaveFavored():
    return 0

def IsWillSaveFavored():
    return 1
def GetSpellListType():
    return spell_list_type_none # dragon disciples only advance bonus spells

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible( alignment):
    return 1

def CanCastInnateArcane(obj): #todo: generalize
    if obj.stat_level_get(stat_level_bard) > 0:
        return 1
    if obj.stat_level_get(stat_level_sorcerer) > 0:
        return 1
    return 0
    
def SpeaksDraconic(obj):
    return 1 # languages not implemented in ToEE

def KnowledgeArcanaCheck( obj ):
    # if (obj.skill_ranks_get(skill_knowledge_arcana) < 8): #knowledge skill not implemented in ToEE
        # return 0
    if obj.stat_level_get(stat_level) < 5: # a replacement for checking knowledge arcana
        return 0
    return 1

def DragonRaceCheck(obj):
    #Can't check if character is half-dragon; it does not exist in ToEE
    if obj.is_category_type(mc_type_dragon):
        return 1
    return 0

def ObjMeetsPrereqs(obj):
    if not KnowledgeArcanaCheck(obj):
        return 0
    elif not CanCastInnateArcane(obj):
        return 0
    elif not SpeaksDraconic(obj):
        return 0
    #elif DragonRaceCheck:
    #    return 0
    return 1


# Levelup

def IsSelectingFeatsOnLevelup(obj):
    return 0

def LevelupGetBonusFeats(obj):
    return

def IsSelectingSpellsOnLevelup( obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    if char_editor.is_selecting_spells(obj, class_extended_1):
        return 1
    return 0


def LevelupCheckSpells(obj, class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    if not char_editor.spells_check_complete(obj, class_extended_1):
        return 0
    return 1

def InitSpellSelection(obj , class_extended_1 = 0):
    newLvl = obj.stat_level_get( classEnum ) + 1
    levelsWithoutNewSpells = [3, 7, 10]
    if not newLvl in levelsWithoutNewSpells:
        if class_extended_1 <= 0:
            class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
        char_editor.init_spell_selection(obj, class_extended_1)
    return 0

def LevelupSpellsFinalize(obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    char_editor.spells_finalize(obj, class_extended_1)
    return 0
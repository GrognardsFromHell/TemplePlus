from toee import *
import char_class_utils
import char_editor
import heritage_feat_utils
###################################################

def GetConditionName():
    return "Dragonheart Mage"

def GetSpellCasterConditionName():
    return "Dragonheart Mage Spellcasting"

def GetCategory():
    return "Races of the Dragon Prestige Classes"

def GetClassDefinitionFlags():
    return CDF_None

def GetClassHelpTopic():
    return "TAG_DRAGONHEART_MAGES"

classEnum = stat_level_dragonheart_mage

###################################################


class_feats = {
1: ("Draconic Breath",),
}

class_skills = (skill_bluff, skill_concentration, skill_gather_information, skill_knowledge_arcana, skill_knowledge_religion, skill_knowledge_nature, skill_listen, skill_search, skill_spellcraft, skill_spot)


def IsEnabled():
    return 1

def GetHitDieType():
    return 6

def GetSkillPtsPerLevel():
    return 2
    
def GetBabProgression():
    return base_attack_bonus_type_non_martial

def IsFortSaveFavored():
    return 1

def IsRefSaveFavored():
    return 0

def IsWillSaveFavored():
    return 1

def GetSpellListType():
    return spell_list_type_extender

def IsClassSkill(skillEnum):
    return char_class_utils.IsClassSkill(class_skills, skillEnum)

def IsClassFeat(featEnum):
    return char_class_utils.IsClassFeat(class_feats, featEnum)

def GetClassFeats():
    return class_feats

def IsAlignmentCompatible(alignment):
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

def hasDraconicHeritageFeat(obj):
    hasFeat = False
    for heritage in range(heritage_draconic_black, heritage_draconic_white + 1):
        colourString = heritage_feat_utils.getDraconicHeritageColourString(heritage)
        if obj.has_feat("Draconic Heritage {}".format(colourString)):
            hasFeat = True
            break
    if not hasFeat:
        return 0
    return 1


def aboveMaxLevelOfPrc(obj):
    #Maximum number of levels for this PrC: 10
    classLvl = obj.stat_level_get(classEnum)
    if classLvl < 10:
        return False
    return True

def ObjMeetsPrereqs(obj):
    if not KnowledgeArcanaCheck(obj):
        return 0
    elif not CanCastInnateArcane(obj):
        return 0
    elif not SpeaksDraconic(obj):
        return 0
    elif not hasDraconicHeritageFeat(obj):
        return 0
    elif aboveMaxLevelOfPrc(obj):
        return 0
    return 1

# Levelup

def IsSelectingFeatsOnLevelup(obj):
    newLvl = char_editor.stat_level_get(classEnum)
    bonusDraconicFeat = [2, 4, 8]
    if newLvl in bonusDraconicFeat:
        return 1
    return 0

def LevelupGetBonusFeats(obj):
    bonFeatInfo = []
    draconicFeats = heritage_feat_utils.getDraconicHeritageFeatList()
    for feat in draconicFeats:
        bonFeatInfo.append(char_editor.FeatInfo(feat))
    char_editor.set_bonus_feats(bonFeatInfo)
    return

def IsSelectingSpellsOnLevelup(obj , class_extended_1 = 0):
    newLvl = char_editor.stat_level_get(classEnum)
    if newLvl == 1 or newLvl == 6:
        return 0
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
    
def InitSpellSelection(obj, class_extended_1 = 0):
    newLvl = char_editor.stat_level_get(classEnum)
    if newLvl == 1 or newLvl == 6:
        return 0
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    char_editor.init_spell_selection(obj, class_extended_1)
    return 0

def LevelupSpellsFinalize(obj, class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    char_editor.spells_finalize(obj, class_extended_1)
    return 0

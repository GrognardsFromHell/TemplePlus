from toee import *
import char_class_utils
import char_editor
import tpdp
###################################################

def GetConditionName():
    return "Unseen Seer"

def GetSpellCasterConditionName():
    return "Unseen Seer Spellcasting"

def GetCategory():
    return "Complete Mage Prestige Classes"

def GetClassDefinitionFlags():
    return CDF_None

def GetClassHelpTopic():
    return "TAG_UNSEEN_SEER"

classEnum = stat_level_unseen_seer

###################################################


class_feats = {
1: ("Unseen Seer Damage Bonus",),
2: ("Unseen Seer Advanced Learning", feat_silent_spell),
3: ("Unseen Seer Divination Spell Power",),
5: ("Guarded Mind",),
}

class_skills = (skill_bluff, skill_concentration, skill_decipher_script, skill_diplomacy, skill_disguise, skill_forgery, skill_gather_information, skill_hide, skill_knowledge_arcana,
skill_knowledge_religion, skill_knowledge_nature, skill_listen, skill_move_silently, skill_profession, skill_search, skill_sense_motive, skill_spellcraft,skill_spot)


def IsEnabled():
    return 1

def GetHitDieType():
    return 4

def GetSkillPtsPerLevel():
    return 6
    
def GetBabProgression():
    return base_attack_bonus_type_semi_martial

def IsFortSaveFavored():
    return 0

def IsRefSaveFavored():
    return 0

def IsWillSaveFavored():
    return 1

# Spell casting
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


def ObjMeetsPrereqs(obj):
    #Maximum number of levels for this PrC: 10
    classLvl = obj.stat_level_get(classEnum)
    if classLvl >= 10:
        return 0

    #Skills: Hide 8 ranks, Search 8 ranks, Sense Motive 4 ranks, Spellcraft 4 ranks, Spot 8 ranks.
    if char_editor.skill_ranks_get(skill_hide) < 8:
        return 0
    elif char_editor.skill_ranks_get(skill_search) < 8:
        return 0
    elif char_editor.skill_ranks_get(skill_sense_motive) < 4:
       return 0
    elif char_editor.skill_ranks_get(skill_spellcraft) < 4:
        return 0
    elif char_editor.skill_ranks_get(skill_spot) < 8:
        return 0

    #Spellcasting: Ability to cast 1st-level arcane spells, including at least two divination spells.
    if obj.arcane_spell_level_can_cast() < 1:
        return 0
    knownSpells = obj.spells_known
    spellCounter = 0
    knowsTwoDivinationSpells = False
    for spell in knownSpells:
        if spell.spell_level > 0:
            spellEntry = tpdp.SpellEntry(spell.spell_enum)
            if spellEntry.spell_school_enum == Divination:
                spellCounter += 1
                if spellCounter == 2:
                    knowsTwoDivinationSpells = True
                    break
    if not knowsTwoDivinationSpells:
        return 0
    return 1

# Levelup

def IsSelectingFeatsOnLevelup(obj):
    return 0

def LevelupGetBonusFeats(obj):
    return

def getAdvancedLearningLevels():
    return [2, 5, 8]

def IsSelectingFeaturesOnLevelup(obj):
    classLevel = char_editor.stat_level_get(classEnum)
    if classLevel in getAdvancedLearningLevels():
        return 1
    return 0

def LevelupFeaturesInit(obj , class_extended_1 = 0):
    print "LevelupFeaturesInit HooK"
    return 0

def LevelupFeaturesFinalize(obj, class_extended_1 = 0):
    print "LevelupFeaturesFinalize Hook"
    return 0

def IsSelectingSpellsOnLevelup(obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    if char_editor.is_selecting_spells(obj, class_extended_1):
        return 1
    return 0

def LevelupCheckSpells(obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    if not char_editor.spells_check_complete(obj, class_extended_1):
        return 0
    return 1
    
def InitSpellSelection(obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    char_editor.init_spell_selection(obj, class_extended_1)
    return 0

def LevelupSpellsFinalize(obj , class_extended_1 = 0):
    if class_extended_1 <= 0:
        class_extended_1 = char_class_utils.GetHighestArcaneClass(obj)
    char_editor.spells_finalize(obj, class_extended_1)
    return 0

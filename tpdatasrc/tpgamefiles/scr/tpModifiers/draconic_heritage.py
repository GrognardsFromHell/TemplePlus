from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import heritage_feat_utils

# Draconic Heritage: Complete Arcane, p. 77

print "Registering Draconic Heritage"

def getDraconicHeritageClassSkill(heritage):
    skillDict = {
    heritage_draconic_black: skill_hide,
    heritage_draconic_blue: skill_listen,
    heritage_draconic_green: skill_move_silently,
    heritage_draconic_red: skill_intimidate,
    heritage_draconic_white: skill_balance,
    heritage_draconic_brass: skill_gather_information,
    heritage_draconic_bronze: skill_wilderness_lore,
    heritage_draconic_copper: skill_hide,
    heritage_draconic_gold: skill_heal,
    heritage_draconic_silver: skill_disguise
    }
    return skillDict[heritage]

def getSaveDescriptor(heritageElement):
    if heritageElement == D20DT_ACID:
        return D20STD_F_SPELL_DESCRIPTOR_ACID
    elif heritageElement == D20DT_COLD:
        return D20STD_F_SPELL_DESCRIPTOR_COLD
    elif heritageElement == D20DT_ELECTRICITY:
        return D20STD_F_SPELL_DESCRIPTOR_ELECTRICITY
    elif heritageElement == D20DT_FIRE:
        return D20STD_F_SPELL_DESCRIPTOR_FIRE
    return 0

def addClassSkill(attachee, args, evt_obj):
    heritage = args.get_arg(1)
    skillEnum = getDraconicHeritageClassSkill(heritage)
    if evt_obj.data1 == skillEnum:
        evt_obj.return_val = 1
    return 0

def addSavingThrowBonus(attachee, args, evt_obj):
    if attachee.stat_level_get(stat_level_sorcerer) > 0:
        heritage = args.get_arg(1)
        heritageElement = heritage_feat_utils.getDraconicHeritageElement(heritage)
        saveDescriptor = getSaveDescriptor(heritageElement)
        flags = evt_obj.flags
        #This is not working properly, because as soon as the spell uses reflex_save_and_damage 
        #the Save Descriptor gets dropped :(
        #I did not use reflex_save_and_damage for my damage spells in the spell compendium
        #but instead the normal saving_throw_spell and then dealt damage afterwards with
        #spell_damage_with_reduction/spell_damage which leads to the situation
        #that the below save bonus works with the spell compendium spells
        #but not with the core spells.
        print "flags: {}".format(flags)
        if (flags & (1 << (saveDescriptor-1))): 
            bonusValue = heritage_feat_utils.countHeritageFeats(attachee, heritage)
            bonusType = 0 # ID 0 = Untyped (stacking)
            evt_obj.bonus_list.add(bonusValue ,bonusType ,"~Draconic Heritage~[TAG_DRACONIC_HERITAGE]")
        #elif #Sleep and Paralyze missing
        #there are no slepp or paralyze flags/descriptors atm, which means, you can do immunities to both types
        #but no save bonus effects
    return 0

def querySelectedHeritage(attachee, args, evt_obj):
    heritage = args.get_arg(1)
    evt_obj.return_val = heritage
    return 0

draconicHeritageFeat = PythonModifier("Draconic Heritage Feat", 3) #featEnum, heritage, empty
for heritage in range(heritage_draconic_black, heritage_draconic_white + 1):
    colourString = heritage_feat_utils.getDraconicHeritageColourString(heritage)
    draconicHeritageFeat.MapToFeat("Draconic Heritage {}".format(colourString), feat_cond_arg2 = heritage)
draconicHeritageFeat.AddHook(ET_OnD20PythonQuery, "PQ_Selected_Draconic_Heritage", querySelectedHeritage, ())
draconicHeritageFeat.AddHook(ET_OnSaveThrowLevel, EK_NONE, addSavingThrowBonus, ())
draconicHeritageFeat.AddHook(ET_OnD20PythonQuery, "Is Class Skill", addClassSkill, ())

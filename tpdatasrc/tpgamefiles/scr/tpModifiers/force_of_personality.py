from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Force of Personality: Complete Adventurer, p. 109

print "Registering Force of Personality"

def applyChaBonus(attachee, args, evt_obj):
    wisMod = (attachee.stat_level_get(stat_wisdom) - 10) / 2
    chaMod = (attachee.stat_level_get(stat_charisma) - 10) /2
    if chaMod > wisMod:
        flags = evt_obj.flags
        if (flags & (1 << (D20STD_F_SPELL_DESCRIPTOR_MIND_AFFECTING-1))): 
            bonus = chaMod - wisMod
            bonusType = 182 #ID 182 = Different Ability Modififers for Will Saves
            evt_obj.bonus_list.add(bonus, bonusType, "~Force of Personality~[TAG_FORCE_OF_PERSONALITY]")
    return 0

forceOfPersonalityFeat= PythonModifier("Force of Personality", 2) #featEnum, empty
forceOfPersonalityFeat.MapToFeat("Force of Personality", feat_cond_arg2 = 0)
forceOfPersonalityFeat.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, applyChaBonus, ())

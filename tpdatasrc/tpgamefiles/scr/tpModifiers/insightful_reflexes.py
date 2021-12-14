from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Insightful Reflexes: Complete Adventurer, p. 110

print "Registering Insightful Reflexes"

def applyIntBonus(attachee, args, evt_obj):
    dexMod = (attachee.stat_level_get(stat_dexterity) - 10) / 2
    intMod = (attachee.stat_level_get(stat_intelligence) - 10) /2
    if intMod > dexMod:
        bonus = intMod - dexMod
        bonusType = 181 #ID 181 = Different Ability Modififers for Reflex Saves
        evt_obj.bonus_list.add(bonus, bonusType, "~Insightful Reflexes~[TAG_INSIGHTFUL_REFLEXES]")
    return 0

insightfulReflexesFeat = PythonModifier("Insightful Reflexes", 2) #featEnum, empty
insightfulReflexesFeat.MapToFeat("Insightful Reflexes", feat_cond_arg2 = 0)
insightfulReflexesFeat.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, applyIntBonus, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import feat_utils

# Steadfast Determination: PHB II, p. 83

print "Registering Steadfast Determination"

def applyConBonus(attachee, args, evt_obj):
    featName = feat_utils.getFeatName(args)
    featTag = feat_utils.getFeatTag(featName)
    wisMod = feat_utils.getAbilityModifier(attachee, stat_wisdom)
    conMod = feat_utils.getAbilityModifier(attachee, stat_constitution)
    if conMod > wisMod:
        bonus = conMod - wisMod
        bonusType = 182 #ID 182 = Different Ability Modififers for Will Saves
        evt_obj.bonus_list.add(bonus, bonusType, "~{}~[{}]".format(featName, featTag))
    return 0

steadfastDeterminationFeat= PythonModifier("Steadfast Determination", 2) #featEnum, empty
steadfastDeterminationFeat.MapToFeat("Steadfast Determination", feat_cond_arg2 = 0)
steadfastDeterminationFeat.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, applyConBonus, ())
steadfastDeterminationFeat.AddHook(ET_OnD20PythonQuery, "PQ_No_Autofail_One_Fortitude_Save", feat_utils.queryReturnOne, ())

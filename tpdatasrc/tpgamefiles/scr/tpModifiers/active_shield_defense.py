from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Active Shield Defense: PHB II, p. 71

def getFeatName():
    return "Active Shield Defense"

print "Registering {}".format(getFeatName)

def getFeatTag(featName):
    return "TAG_{}".format(featName.upper().replace(" ", "_"))

def nullifyToHitPenalty(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if not flags & D20CAF_ATTACK_OF_OPPORTUNITY:
        return 0
    elif not attachee.d20_query("Fighting Defensively Checked"):
        return 0
    elif attachee.item_worn_at(item_wear_shield) == OBJ_HANDLE_NULL:
        return 0
    elif attachee.d20_query("PQ_Total_Defense_Activated"):
        return 0
    else:
        featName = getFeatName()
        featTag = getFeatTag(featName)
        #Fighting Defensively has a vanilla bonus type of 0, which means it can't be negated
        #So I add the penalty as a bonus again
        bonusValue = 4
        bonusType = 0 #ID 0 = Untyped (stacking!)
        evt_obj.bonus_list.add(bonusValue, bonusType, "Feat: ~{}~[{}]".format(featName, featTag))
    return 0

# Allowing AoO's while in Total Defense is directly handled in
# total_defense.py (an already existing PythonModifier Extender)

activeShieldDefenseFeat = PythonModifier(getFeatName(), 2) #featEnum, empty
activeShieldDefenseFeat.MapToFeat(getFeatName(), feat_cond_arg2 = 0)
activeShieldDefenseFeat.AddHook(ET_OnToHitBonus2, EK_NONE, nullifyToHitPenalty, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Shield Specialization: PHB II, p. 82

def getFeatName():
    return "Shield Specialization"

print "Registering {}".format(getFeatName)

def getFeatTag(featName):
    return "TAG_{}".format(featName.upper().replace(" ", "_"))

#def getSubFeatList():
#    return {0:"Buckler", 1:"Heavy", 2:"Light"}

def applyBonus(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    if attachee.item_worn_at(item_wear_shield) == OBJ_HANDLE_NULL:
        return 0
    elif flags & D20CAF_TOUCH_ATTACK:
        return 0
    else:
        featName = getFeatName()
        featTag = getFeatTag(featName)
        bonusValue = 1
        bonusType = 0 #ID 0 = Untyped (stacking!)
        evt_obj.bonus_list.add(bonusValue, bonusType, "Feat: ~{}~[{}]".format(featName, featTag))
    return 0

shieldSpecializationFeat = PythonModifier(getFeatName(), 2) #featEnum, empty
shieldSpecializationFeat.MapToFeat(getFeatName(), feat_cond_arg2 = 0)
shieldSpecializationFeat.AddHook(ET_OnGetAC, EK_NONE, applyBonus, ())

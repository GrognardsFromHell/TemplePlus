from templeplus.pymod import PythonModifier
from toee import *
import tpactions

# Shield Ward: PHB II, p. 82

def getFeatName():
    return "Shield Ward"

print "Registering {}".format(getFeatName)

def getFeatTag(featName):
    return "TAG_{}".format(featName.upper().replace(" ", "_"))

def getShieldBonus(wornShield):
    bonusValue = wornShield.item_d20_query(Q_Armor_Get_AC_Bonus)
    bonusValue += 1 #Add Shield Specialization Bonus as well; SP is a prereq for this feat
    return bonusValue

def applyBonus(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    wornShield = attachee.item_worn_at(item_wear_shield)
    if wornShield == OBJ_HANDLE_NULL:
        return 0
    elif not flags & D20CAF_TOUCH_ATTACK:
        return 0
    else:
        featName = getFeatName()
        featTag = getFeatTag(featName)
        bonusValue = getShieldBonus(wornShield)
        bonusType = 0 #ID 0 = Untyped (stacking!)
        evt_obj.bonus_list.add(bonusValue, bonusType, "Feat: ~{}~[{}]".format(featName, featTag))
    return 0

def addBonusAgainstSpecialAttacks(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    performer = currentSequence.performer
    if performer != attachee:
        wornShield = attachee.item_worn_at(item_wear_shield)
        flags = evt_obj.flags
        if wornShield != OBJ_HANDLE_NULL and flags == 3:
            featName = getFeatName()
            featTag = getFeatTag(featName)
            bonusValue = getShieldBonus(wornShield)
            bonusType = 0 #ID 0 = Untyped (stacking!)
            evt_obj.bonus_list.add(bonusValue, bonusType, "Feat: ~{}~[{}]".format(featName, featTag))
    return 0

shieldWardFeat = PythonModifier(getFeatName(), 2) #featEnum, empty
shieldWardFeat.MapToFeat(getFeatName(), feat_cond_arg2 = 0)
shieldWardFeat.AddHook(ET_OnGetAC, EK_NONE, applyBonus, ())
shieldWardFeat.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, addBonusAgainstSpecialAttacks, ())

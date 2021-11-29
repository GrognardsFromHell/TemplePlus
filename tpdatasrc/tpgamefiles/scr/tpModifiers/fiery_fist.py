from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fiery Fist: Player Handbook II, p. 79

fieryFistEnum = 1222

def getFeatName():
    return "Fiery Fist"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def addExtraCharges(attachee, args, evt_obj):
    evt_obj.return_val += 1
    return 0

def createRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, fieryFistEnum, radialData1, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def actionCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    if not attachee.item_worn_at(item_wear_weapon_primary) == OBJ_HANDLE_NULL:
        evt_obj.return_val = AEC_WRONG_WEAPON_TYPE
    return 0

def actionPerform(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    particlesString = "ft-{}".format(featName)
    particlesID = game.particles(particlesString, attachee)
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
    conditionName = "{} Effect".format(featName)
    if attachee.condition_add_with_args(conditionName, particlesID, 0):
        # Deduct Stunning Fist Charge
        chargesToDeduct = 1
        attachee.d20_send_signal("PS_Deduct_Stunning_Fist_Charge", chargesToDeduct)
    return 0

fieryFistFeat = PythonModifier("{} Feat".format(getFeatName()), 2) #featEnum, empty
fieryFistFeat.MapToFeat("{}".format(getFeatName()), feat_cond_arg2 = 0)
fieryFistFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, createRadial, ())
fieryFistFeat.AddHook(ET_OnD20PythonActionCheck, fieryFistEnum, actionCheck, ())
fieryFistFeat.AddHook(ET_OnD20PythonActionPerform, fieryFistEnum, actionPerform, ())
fieryFistFeat.AddHook(ET_OnD20PythonQuery, "PQ_Get_Extra_Stunning_Fist_Charges", addExtraCharges, ())

# Fiery Fist Effect
def addExtraDamage(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if weaponUsed == OBJ_HANDLE_NULL:
        damageDice = dice_new('1d6')
        damageType = D20DT_FIRE
        damageMesId = 4003 #NEW ID! added in damage_ext.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def removeOnBeginRound(attachee, args, evt_obj):
    args.condition_remove()
    return 0

def removeParticles(attachee, args, evt_obj):
    particlesID = args.get_arg(0)
    game.particles_end(particlesID)
    return 0

def tooltip(attachee, args, evt_obj):
    featName = getFeatName()
    evt_obj.append("{} activated".format(featName))
    return 0

fieryFistEffect = PythonModifier("{} Effect".format(getFeatName()), 2) #particlesID, empty
fieryFistEffect.AddHook(ET_OnDealingDamage, EK_NONE, addExtraDamage, ())
fieryFistEffect.AddHook(ET_OnBeginRound, EK_NONE, removeOnBeginRound, ())
fieryFistEffect.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
fieryFistEffect.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

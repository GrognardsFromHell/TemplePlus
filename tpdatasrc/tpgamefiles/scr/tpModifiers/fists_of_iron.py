from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fists of Iron: Complete Warrior, p. 99

fistsOfIronEnum = 1220

def getFeatName():
    return "Fists of Iron"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def fistsOfIronRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, fistsOfIronEnum, radialData1, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def fistsOfIronCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def fistsOfIronPerform(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    particlesString = "ft-{}".format(featName)
    particlesID = game.particles(particlesString, attachee)
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
    conditionName = "{} Effect".format(featName)
    attachee.condition_add_with_args(conditionName, particlesID, 0)
    return 0

fistsOfIronFeat = PythonModifier("{} Feat".format(getFeatName()), 2) #featEnum, empty
fistsOfIronFeat.MapToFeat("{}".format(getFeatName()), feat_cond_arg2 = 0)
fistsOfIronFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, fistsOfIronRadial, ())
fistsOfIronFeat.AddHook(ET_OnD20PythonActionCheck, fistsOfIronEnum, fistsOfIronCheck, ())
fistsOfIronFeat.AddHook(ET_OnD20PythonActionPerform, fistsOfIronEnum, fistsOfIronPerform, ())

# Fists of Iron Effect
def addExtraDamage(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if weaponUsed == OBJ_HANDLE_NULL:
        damageDice = dice_new('1d6')
        damageType = D20DT_UNSPECIFIED #Damage Type will be determinated by original attack
        damageMesId = 4001 #ID 4001 NEW! added in damage_ext.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def removeSignal(attachee, args, evt_obj):
    # Deduct Stunning Fist Charge
    chargesToDeduct = 1
    attachee.d20_send_signal("PS_Deduct_Stunning_Fist_Charge", chargesToDeduct)
    # Remove after use
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

fistsOfIronEffect = PythonModifier("{} Effect".format(getFeatName()), 2) #particlesID, empty
fistsOfIronEffect.AddHook(ET_OnDealingDamage, EK_NONE, addExtraDamage, ())
fistsOfIronEffect.AddHook(ET_OnD20Signal, EK_S_Attack_Made, removeSignal, ())
fistsOfIronEffect.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
fistsOfIronEffect.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

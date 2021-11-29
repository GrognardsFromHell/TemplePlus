from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Axiomatic Strike: Complete Warrior, p. 96

axiomaticStrikeEnum = 1221

def getFeatName():
    return "Axiomatic Strike"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def createRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, axiomaticStrikeEnum, radialData1, featTag)
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

axiomaticStrikeFeat = PythonModifier("{} Feat".format(getFeatName()), 2) #featEnum, empty
axiomaticStrikeFeat.MapToFeat("{}".format(getFeatName()), feat_cond_arg2 = 0)
axiomaticStrikeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, createRadial, ())
axiomaticStrikeFeat.AddHook(ET_OnD20PythonActionCheck, axiomaticStrikeEnum, actionCheck, ())
axiomaticStrikeFeat.AddHook(ET_OnD20PythonActionPerform, axiomaticStrikeEnum, actionPerform, ())

# Axiomatic Strike Effect
def addExtraDamage(attachee, args, evt_obj):
    target = evt_obj.attack_packet.target
    targetAlignment = target.critter_get_alignment()
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if weaponUsed == OBJ_HANDLE_NULL and targetAlignment & ALIGNMENT_CHAOTIC:
        damageDice = dice_new('1d6')
        damageDice.number = 2
        damageType = D20DT_UNSPECIFIED #Damage Type will be determinated by original attack
        damageMesId = 4002 #ID 4002 NEW! added in damage_ext.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def removeSignal(attachee, args, evt_obj):
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

axiomaticStrikeEffect = PythonModifier("{} Effect".format(getFeatName()), 2) #particlesID, empty
axiomaticStrikeEffect.AddHook(ET_OnDealingDamage, EK_NONE, addExtraDamage, ())
axiomaticStrikeEffect.AddHook(ET_OnD20Signal, EK_S_Attack_Made, removeSignal, ())
axiomaticStrikeEffect.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
axiomaticStrikeEffect.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Fiery Ki Defense: Player Handbook II, p. 79

fieryKiDefenseEnum = 1223

def getFeatName():
    return "Fiery Ki Defense"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def createRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, fieryKiDefenseEnum, radialData1, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def actionCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
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
fieryFistFeat.AddHook(ET_OnD20PythonActionCheck, fieryKiDefenseEnum, actionCheck, ())
fieryFistFeat.AddHook(ET_OnD20PythonActionPerform, fieryKiDefenseEnum, actionPerform, ())

# Fiery Ki Defense Effect
def dealDamageWhenHit(attachee, args, evt_obj):
    target = evt_obj.attack_packet.attacker
    featName = getFeatName()
    damageDice = dice_new('1d6')
    damageType = D20DT_FIRE
    target.damage(attachee, damageType, damageDice, D20DAP_UNSPECIFIED, D20A_NONE)
    attachee.float_text_line("{}".format(featName))
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
    evt_obj.append("{}".format(featName))
    return 0

fieryFistEffect = PythonModifier("{} Effect".format(getFeatName()), 2) #particlesID, empty
fieryFistEffect.AddHook(ET_OnTakingDamage, EK_NONE, dealDamageWhenHit, ())
fieryFistEffect.AddHook(ET_OnBeginRound, EK_NONE, removeOnBeginRound, ())
fieryFistEffect.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
fieryFistEffect.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

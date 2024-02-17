from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Shield Charge: Complete Warrior, p. 105

def getFeatName():
    return "Shield Charge"

print "Registering {}".format(getFeatName)

def getFeatTag(featName):
    return "TAG_{}".format(featName.upper().replace(" ", "_"))

def radialEntry(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag(featName)
    toggleRadialId = tpdp.RadialMenuEntryToggle(featName, featTag)
    toggleRadialId.link_to_args(args, 1)
    toggleRadialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Options)
    return 0

def tripAttempt(attachee, args, evt_obj):
    enabledFlag = args.get_arg(1)
    if enabledFlag and attachee.d20_query("Charging"):
        if attachee.item_worn_at(item_wear_shield) != OBJ_HANDLE_NULL:
            target = evt_obj.attack_packet.target
            if attachee.trip_check(target):
                combatMesId = 104 # ID 104 = Tripped!
                target.float_mesfile_line('mes\\combat.mes', combatMesId)
                target.condition_add_with_args("Prone")
                target.fall_down()
            else:
                combatMesId = 144 #ID 144 = Attempt Fails
                attachee.float_mesfile_line('mes\\combat.mes', combatMesId)
    return 0

shieldChargeFeat = PythonModifier(getFeatName(), 3) #featEnum, enabledFlag, empty
shieldChargeFeat.MapToFeat(getFeatName(), feat_cond_arg2 = 1)
shieldChargeFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialEntry, ())
shieldChargeFeat.AddHook(ET_OnDealingDamage2, EK_NONE, tripAttempt, ())

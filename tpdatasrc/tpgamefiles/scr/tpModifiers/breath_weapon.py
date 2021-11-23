from templeplus.pymod import PythonModifier
from toee import *
import tpdp

###################################################

print "Registering Breath Weapon"

###################################################

def breathWeaponOnConditionAdd(attachee, args, evt_obj):
    breathWeaponId = args.get_param(0)
    breathWeaponCharges = args.get_param(1)
    cooldown = 0
    args.set_arg(0, breathWeaponId)
    args.set_arg(1, breathWeaponCharges)
    args.set_arg(2, cooldown)
    args.set_arg(3, 0)
    args.set_arg(4, 0)
    return 0

# Handle Breath Weapon Charges
def addExtraExhalationCharges(attachee):
    extraBreathExhalation = attachee.has_feat("Extra Exhalation")
    return extraBreathExhalation

def getMaxCharges(attachee):
    maxCharges = 1 + addExtraExhalationCharges(attachee)
    return maxCharges

def resetBreathWeaponUses(attachee, args, evt_obj):
    if not args.get_arg(1) == -1:
        maxCharges = getMaxCharges(attachee)
        args.set_arg(1, maxCharges)
    return 0

# Handle Breath Weapon Cooldown
def getBreathWeaponCoolDown():
    cooldownDice = dice_new('1d4')
    return cooldownDice.roll()

def reduceBreathWeaponCooldown(attachee, args, evt_obj):
    if args.get_arg(2) > -1:
        cooldown = args.get_arg(2)
        cooldown -= evt_obj.data1
        if cooldown < 0 and game.combat_is_active():
            attachee.float_text_line("Breath Weapon ready")
        args.set_arg(2, cooldown)
    return 0

# Trigger Breath Weapon Used
def signalBreathWeaponUsed(attachee, args, evt_obj):
    signalId = evt_obj.data1
    breathWeaponId = args.get_arg(0)
    if signalId == breathWeaponId:
        breathWeaponCharges = args.get_arg(1)
        if not breathWeaponCharges == 1:
            cooldown = getBreathWeaponCoolDown()
            args.set_arg(2, cooldown)
        if not breathWeaponCharges == -1:
            breathWeaponCharges -= 1
            args.set_arg(1, breathWeaponCharges)
    return 0

class BreathWeaponModifier(PythonModifier):
    # Breath Weapon modifiers have 5 arguments:
    # 0: breathWeaponId, 1: charges, 2: Cooldown, 3: empty, 4: empty
    # Charges set to -1 indicates limitless breath weapon uses
    # A Breath Weapon usage always triggers a 1d4 long cooldown
    # before the Breath Weapon becomes availible again
    def __init__(self, name):
        PythonModifier.__init__(self, name, 5, True)
        self.AddHook(ET_OnD20PythonSignal, "PS_Breath_Weapon_Used", signalBreathWeaponUsed, ())
        self.AddHook(ET_OnBeginRound, EK_NONE, reduceBreathWeaponCooldown, ())
        self.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetBreathWeaponUses, ())

    # This hook needs to be added for every BreathWeaponModifier
    def breathWeaponSetArgs(self, breathWeaponId, breathWeaponCharges):
        self.AddHook(ET_OnConditionAdd, EK_NONE, breathWeaponOnConditionAdd, (breathWeaponId, breathWeaponCharges,))


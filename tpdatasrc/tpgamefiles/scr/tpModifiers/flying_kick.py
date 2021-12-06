from templeplus.pymod import PythonModifier
from toee import *

# Flying Kick: Complete Warrior, p. 99

print "Registering Flying Kick"

def flyingKickExtraDamage(attachee, args, evt_obj):
    flags = evt_obj.attack_packet.get_flags()
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if flags & D20CAF_CHARGE and weaponUsed == OBJ_HANDLE_NULL:
        damageDice = dice_new('1d12')
        damageType = D20DT_UNSPECIFIED #Damage Type will be determinated by original attack
        damageMesId = 4000 #ID 4000 NEW! added in damage_ext.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

flyingKickFeat = PythonModifier("Flying Kick Feat", 2) #featEnum, empty
flyingKickFeat.MapToFeat("Flying Kick", feat_cond_arg2 = 0)
flyingKickFeat.AddHook(ET_OnDealingDamage, EK_NONE, flyingKickExtraDamage, ())

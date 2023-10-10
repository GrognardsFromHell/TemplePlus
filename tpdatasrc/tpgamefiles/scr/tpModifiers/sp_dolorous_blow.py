from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Dolorous Blow"

def dolorousBlowSpellOnConditionAdd(attachee, args, evt_obj):
    attachee.item_condition_add_with_args('Weapon Dolorous Blow', 0, 0, 0, 0, args.get_arg(0))
    return 0

def dolorousBlowSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Dolorous Blow', args.get_arg(0))
    return 0

dolorousBlowSpell = PythonModifier("sp-Dolorous Blow", 3) # spell_id, duration, empty
dolorousBlowSpell.AddHook(ET_OnConditionAdd, EK_NONE, dolorousBlowSpellOnConditionAdd,())
dolorousBlowSpell.AddHook(ET_OnConditionRemove, EK_NONE, dolorousBlowSpellWeaponConditionRemove,())
dolorousBlowSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
dolorousBlowSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
dolorousBlowSpell.AddSpellDispelCheckStandard()
dolorousBlowSpell.AddSpellTeleportPrepareStandard()
dolorousBlowSpell.AddSpellTeleportReconnectStandard()
dolorousBlowSpell.AddSpellCountdownStandardHook()

###### Dolorous Blow Weapon Condition ######

def weaponDolorousBlowModifyThreatRange(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        bonusValue = usedWeapon.obj_get_int(obj_f_weapon_crit_range) #Dolorous Blow doubles Threat Range
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Dolorous Blow~[TAG_SPELLS_DOLORUS_BLOW] Bonus")
    return 0

def weaponDolorousBlowAutoConfirmCrit(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 1
    return 0

def weaponDolorousBlowGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 1
    return 1

weaponDolorousBlow = PythonModifier("Weapon Dolorous Blow", 5) # empty, empty, inventoryLocation, empty, spell_id
weaponDolorousBlow.AddHook(ET_OnGetCriticalHitRange, EK_NONE, weaponDolorousBlowModifyThreatRange,())
weaponDolorousBlow.AddHook(ET_OnD20PythonQuery, "Always Confirm Criticals", weaponDolorousBlowAutoConfirmCrit,())
weaponDolorousBlow.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponDolorousBlowGlowEffect, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Deafening Clang"

def deafeningClangSpellOnConditionAdd(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Deafening Clang', 0, args.get_arg(2), 0, 0, args.get_arg(0))
    return 0

def deafeningClangSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Deafening Clang', args.get_arg(0))
    return 0

deafeningClangSpell = PythonModifier("sp-Deafening Clang", 4) # spell_id, duration, spellDc, empty
deafeningClangSpell.AddHook(ET_OnConditionAdd, EK_NONE, deafeningClangSpellOnConditionAdd,())
deafeningClangSpell.AddHook(ET_OnConditionRemove, EK_NONE, deafeningClangSpellWeaponConditionRemove, ())
deafeningClangSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
deafeningClangSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
deafeningClangSpell.AddSpellDispelCheckStandard()
deafeningClangSpell.AddSpellTeleportPrepareStandard()
deafeningClangSpell.AddSpellTeleportReconnectStandard()
deafeningClangSpell.AddSpellCountdownStandardHook()

###### Deafening Clang Weapon Condition ######

def weaponDeafeningClangOnDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0

    target = evt_obj.attack_packet.target
    spellPacket = tpdp.SpellPacket(args.get_arg(4))

    #Apply extra sonic damage
    damageDice = dice_new('1d6')
    damageType = D20DT_SONIC
    damageMesId = 3004 #ID 3001 added in damage.mes
    evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)

    #Handle deafened part of the spell
    targetIsAlreadyDeaf = target.d20_query(Q_Critter_Is_Deafened)
    if targetIsAlreadyDeaf:
        return 0
    if target.saving_throw_spell(args.get_arg(1), D20_Save_Fortitude, D20STD_F_NONE, spellPacket.caster, args.get_arg(4)): #success
        target.float_mesfile_line('mes\\spell.mes', 30001)
    else:
        target.float_mesfile_line('mes\\spell.mes', 30002)
        if target.condition_add_with_args('sp-Deafness', 0, 10, 0): #sp-Deafness is permanent, no matter what duration you pass.
            game.particles('sp-Blindness-Deafness', target)
            target.condition_add_with_args('Deafening Clang Remove', 10) #helper condition to remove deafness after 1 min
        else:
            game.particles('Fizzle', target)
    return 0

def weaponDeafeningClangGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 7 #There is no sonic weapon glow effect, using holy
    return 1

weaponDeafeningClang = PythonModifier("Weapon Deafening Clang", 5) # empty, spellDc, inventoryLocation, empty, spell_id
weaponDeafeningClang.AddHook(ET_OnDealingDamage, EK_NONE, weaponDeafeningClangOnDamage,())
weaponDeafeningClang.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponDeafeningClangGlowEffect, ())

##### Deafening Clang Remove Deafness Helper Condition #####

def helperDeafeningClangOnRemove(attachee, args, evt_obj):
    attachee.condition_add_with_args('sp-Remove Deafness', 0, 0, 0)
    return 0

def helperDeafeningClangTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

helperDeafeningClang = PythonModifier("Deafening Clang Remove", 1) #duration
helperDeafeningClang.AddHook(ET_OnConditionRemove, EK_NONE, helperDeafeningClangOnRemove, ())
helperDeafeningClang.AddHook(ET_OnBeginRound, EK_NONE, helperDeafeningClangTickdown, ())
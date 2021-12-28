from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Align Weapon"

def alignWeaponSpellOnConditionAdd(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Align', 0, args.get_arg(2), 0, 0, args.get_arg(0))
    return 0

def alignWeaponSpellOnConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Align', args.get_arg(0))
    return 0

alignWeaponSpell = PythonModifier("sp-Align Weapon", 4) # spell_id, duration, alignType, empty
alignWeaponSpell.AddHook(ET_OnConditionAdd, EK_NONE, alignWeaponSpellOnConditionAdd,())
alignWeaponSpell.AddHook(ET_OnConditionRemove, EK_NONE, alignWeaponSpellOnConditionRemove, ())
alignWeaponSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
alignWeaponSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
alignWeaponSpell.AddSpellDispelCheckStandard()
alignWeaponSpell.AddSpellTeleportPrepareStandard()
alignWeaponSpell.AddSpellTeleportReconnectStandard()
alignWeaponSpell.AddSpellCountdownStandardHook()


###### Align Weapon Condition ######

def weaponAlignOnDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    #Check if weapon is already aligned
    if evt_obj.damage_packet.attack_power & D20DAP_HOLY:
        return 0
    elif evt_obj.damage_packet.attack_power & D20DAP_UNHOLY:
        return 0
    elif evt_obj.damage_packet.attack_power & D20DAP_LAW:
        return 0
    elif evt_obj.damage_packet.attack_power & D20DAP_CHAOS:
        return 0

    alignType = args.get_arg(1)

    if alignType == 1:
        evt_obj.damage_packet.attack_power |= D20DAP_HOLY
        game.particles('hit-HOLY-medium', evt_obj.attack_packet.target)
    elif alignType == 2:
        evt_obj.damage_packet.attack_power |= D20DAP_UNHOLY
        game.particles('hit-UNHOLY-medium', evt_obj.attack_packet.target)
    elif alignType == 3:
        evt_obj.damage_packet.attack_power |= D20DAP_LAW
        game.particles('hit-LAW-medium', evt_obj.attack_packet.target)
    elif alignType == 4:
        evt_obj.damage_packet.attack_power |= D20DAP_CHAOS
        game.particles('hit-CHAOTIC-medium', evt_obj.attack_packet.target)
    return 0

def weaponAlignGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    alignType = args.get_arg(1)
    if alignType == 1:
        evt_obj.return_val = 7
    elif alignType == 2:
        evt_obj.return_val = 10
    elif alignType == 3:
        evt_obj.return_val = 8
    elif alignType == 4:
        evt_obj.return_val = 4
    return 0

weaponAlign = PythonModifier("Weapon Align", 5) # empty, alignType, inventoryLocation, empty, spell_id
weaponAlign.AddHook(ET_OnDealingDamage, EK_NONE, weaponAlignOnDamage,())
weaponAlign.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponAlignGlowEffect, ())

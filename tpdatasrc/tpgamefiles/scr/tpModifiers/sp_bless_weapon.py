from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Bless Weapon"

def blessWeaponSpellOnConditionAdd(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Bless', 0, 0, 0, 0, args.get_arg(0))
    return 0

def blessWeaponSpellConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Bless', args.get_arg(0))
    return 0

blessWeaponSpell = PythonModifier("sp-Bless Weapon", 3) # spell_id, duration, empty
blessWeaponSpell.AddHook(ET_OnConditionAdd, EK_NONE, blessWeaponSpellOnConditionAdd,())
blessWeaponSpell.AddHook(ET_OnConditionRemove, EK_NONE, blessWeaponSpellConditionRemove, ())
blessWeaponSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
blessWeaponSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
blessWeaponSpell.AddSpellDispelCheckStandard()
blessWeaponSpell.AddSpellTeleportPrepareStandard()
blessWeaponSpell.AddSpellTeleportReconnectStandard()
blessWeaponSpell.AddSpellCountdownStandardHook()

###### Bless Weapon Condition ######

def weaponBlessOnDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    targetAlignment = evt_obj.attack_packet.target.critter_get_alignment()
    if not evt_obj.damage_packet.attack_power & D20DAP_HOLY:
        evt_obj.damage_packet.attack_power |= D20DAP_HOLY
        game.particles('hit-HOLY-medium', evt_obj.attack_packet.target)
    if targetAlignment & ALIGNMENT_EVIL:
        if not evt_obj.damage_packet.attack_power & D20DAP_MAGIC:
            evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
    return 0

def weaponBlessCheckThreatRange(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    appliedKeenRange = evt_obj.bonus_list.get_sum()
    weaponKeenRange = usedWeapon.obj_get_int(obj_f_weapon_crit_range)
    if appliedKeenRange == weaponKeenRange:
        args.set_arg(1, 0)
    else:
        args.set_arg(1, 1)
    return 0

def weaponBlessConfirmCrit(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    targetAlignment = evt_obj.attack_packet.target.critter_get_alignment()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    if not targetAlignment & ALIGNMENT_EVIL:
        return 0
    if args.get_arg(1):
        return 0
    flags = evt_obj.attack_packet.get_flags()
    flags |= D20CAF_CRITICAL
    evt_obj.attack_packet.set_flags(flags)
    return 0

def weaponBlessGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 7
    return 0

weaponBless = PythonModifier("Weapon Bless", 5) # empty, keenFlag, inventoryLocation, empty, spell_id
weaponBless.AddHook(ET_OnDealingDamage, EK_NONE, weaponBlessOnDamage,())
weaponBless.AddHook(ET_OnGetCriticalHitRange, EK_NONE, weaponBlessCheckThreatRange,())
weaponBless.AddHook(ET_OnConfirmCriticalBonus, EK_NONE, weaponBlessConfirmCrit,())
weaponBless.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponBlessGlowEffect, ())

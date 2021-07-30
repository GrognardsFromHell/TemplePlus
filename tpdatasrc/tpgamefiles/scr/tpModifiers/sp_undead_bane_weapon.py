from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Undead Bane Weapon"

def undeadBaneWeaponSpellAddWeaponCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Undead Bane', 0, 0, 0, 0, args.get_arg(0))
    return 0

def undeadBaneWeaponSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Undead Bane', args.get_arg(0))
    return 0

undeadBaneWeaponSpell = PythonModifier("sp-Undead Bane Weapon", 3) # spell_id, duration, empty
undeadBaneWeaponSpell.AddHook(ET_OnConditionAdd, EK_NONE, undeadBaneWeaponSpellAddWeaponCondition,())
undeadBaneWeaponSpell.AddHook(ET_OnConditionRemove, EK_NONE, undeadBaneWeaponSpellWeaponConditionRemove, ())
undeadBaneWeaponSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
undeadBaneWeaponSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
undeadBaneWeaponSpell.AddSpellDispelCheckStandard()
undeadBaneWeaponSpell.AddSpellTeleportPrepareStandard()
undeadBaneWeaponSpell.AddSpellTeleportReconnectStandard()
undeadBaneWeaponSpell.AddSpellCountdownStandardHook()

###### Undead Bane Weapon Condition ######

def weaponUndeadBaneBonusToHit(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        target = evt_obj.attack_packet.target
        if target.is_category_type(mc_type_undead):
            bonusValue = 2 #Undead Bane Weapon grants an additional (stacking) +2 enhancement bonus if target is undead; I dont think I can query a weapons enhancement bonus so I will simply add +2 stacking
            bonusType = 0 #ID 0 = Untyped (stacking)
            evt_obj.bonus_list.add(bonusValue, bonusType, "~Undead Bane Weapon~[TAG_SPELLS_UNDEAD_BANE_WEAPON] Bonus")

    return 0

def weaponUndeadBaneOnDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        target = evt_obj.attack_packet.target
        #Add Holy property
        if not evt_obj.damage_packet.attack_power & D20DAP_HOLY:
            evt_obj.damage_packet.attack_power |= D20DAP_HOLY
            if not target.is_category_type(mc_type_undead):
                game.particles('hit-HOLY-medium', evt_obj.attack_packet.target)
        #Add bonus damage if target is undead (2d6)
        if target.is_category_type(mc_type_undead):
            if not evt_obj.damage_packet.attack_power & D20DAP_MAGIC:
                evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
            damageDice = dice_new('1d6')
            damageDice.number = 2
            damageType = D20DT_UNSPECIFIED
            damageMesId = 3006 #ID3006 added in damage.mes
            evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
            game.particles('hit-BANE-medium', evt_obj.attack_packet.target)
    return 0

def undeadBaneWeaponConditionGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 7 #ID 7 = Holy
    return 1

weaponUndeadBane = PythonModifier("Weapon Undead Bane", 5) # empty, empty, inventoryLocation, empty, spell_id
weaponUndeadBane.AddHook(ET_OnToHitBonus2, EK_NONE, weaponUndeadBaneBonusToHit, ())
weaponUndeadBane.AddHook(ET_OnDealingDamage, EK_NONE, weaponUndeadBaneOnDamage, ())
weaponUndeadBane.AddHook(ET_OnWeaponGlowType, EK_NONE, undeadBaneWeaponConditionGlowEffect, ())

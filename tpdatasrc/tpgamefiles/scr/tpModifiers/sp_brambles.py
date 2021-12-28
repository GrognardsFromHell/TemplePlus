from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Brambles"

def bramblesSpellAddWeaponCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Brambles', 0, args.get_arg(2), 0, 0, args.get_arg(0))
    return 0

def bramblesSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Brambles', args.get_arg(0))
    return 0

bramblesSpell = PythonModifier("sp-Brambles", 4) # spell_id, duration, bonusDamage, empty
bramblesSpell.AddHook(ET_OnConditionAdd, EK_NONE, bramblesSpellAddWeaponCondition,())
bramblesSpell.AddHook(ET_OnConditionRemove, EK_NONE, bramblesSpellWeaponConditionRemove, ())
bramblesSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
bramblesSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
bramblesSpell.AddSpellDispelCheckStandard()
bramblesSpell.AddSpellTeleportPrepareStandard()
bramblesSpell.AddSpellTeleportReconnectStandard()
bramblesSpell.AddSpellCountdownStandardHook()

###### Brambles Weapon Condition ######

def bramblesWeaponConditionToHitBonus(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        bonusValue = 1 #Brambles grants a +1 enhancement bonus to the weapon
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Brambles~[TAG_SPELLS_BRAMBLES]")
    return 0

def bramblesWeaponConditionBonusToDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if not spell_utils.verifyItem(usedWeapon, args):
        return 0
    weaponDamageType = attachee.obj_get_int(obj_f_weapon_attacktype)
    if not weaponDamageType == D20DT_BLUDGEONING_AND_PIERCING:
        if not weaponDamageType == D20DT_BLUDGEONING:
            evt_obj.damage_packet.attack_power |= D20DAP_BLUDGEONING
        if not weaponDamageType == D20DT_PIERCING:
            evt_obj.damage_packet.attack_power |= D20DAP_PIERCING
    if not evt_obj.damage_packet.attack_power & D20DAP_MAGIC:
        evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
    bonusValue = args.get_arg(1) #Brambles grants a bonus equal to the casterlevel on damage (caps at 10)
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.damage_packet.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Brambles~[TAG_SPELLS_BRAMBLES]")
    return 0

def bramblesWeaponConditionGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        if not evt_obj.return_val:
            evt_obj.return_val = 1
    return 0

weaponBrambles = PythonModifier("Weapon Brambles", 5) # empty, bonusDamage, inventoryLocation, empty, spell_id
weaponBrambles.AddHook(ET_OnToHitBonus2, EK_NONE, bramblesWeaponConditionToHitBonus, ())
weaponBrambles.AddHook(ET_OnDealingDamage, EK_NONE, bramblesWeaponConditionBonusToDamage,())
weaponBrambles.AddHook(ET_OnWeaponGlowType, EK_NONE, bramblesWeaponConditionGlowEffect, ())

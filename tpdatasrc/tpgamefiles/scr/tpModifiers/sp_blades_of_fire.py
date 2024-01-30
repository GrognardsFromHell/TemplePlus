from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Blades of Fire"

def bladesOfFireSpellAddWeaponCondition(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Blades of Fire', 0, 0, 0, 0, spellId)
    attachee.item_condition_add_with_args('Weapon Blades of Fire Tooltip', 0, 0, 0, 0, spellId)
    return 0

def bladesOfFireSpellDurationQuery(attachee, args, evt_obj):
    if args.get_arg(0) == evt_obj.data1:
        evt_obj.return_val = args.get_arg(1)
    else:
        evt_obj.return_val = 0
    return 0

def bladesOfFireSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Blades of Fire', args.get_arg(0))
    attachee.item_condition_remove('Weapon Blades of Fire Tooltip', args.get_arg(0))
    return 0

bladesOfFireSpell = PythonModifier("sp-Blades of Fire", 3) # spell_id, duration, empty
bladesOfFireSpell.AddHook(ET_OnConditionAdd, EK_NONE, bladesOfFireSpellAddWeaponCondition,())
bladesOfFireSpell.AddHook(ET_OnD20PythonQuery, "PQ_Item_Buff_Duration", bladesOfFireSpellDurationQuery, ())
bladesOfFireSpell.AddHook(ET_OnConditionRemove, EK_NONE, bladesOfFireSpellWeaponConditionRemove, ())
bladesOfFireSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
bladesOfFireSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
bladesOfFireSpell.AddSpellDispelCheckStandard()
bladesOfFireSpell.AddSpellTeleportPrepareStandard()
bladesOfFireSpell.AddSpellTeleportReconnectStandard()
bladesOfFireSpell.AddSpellCountdownStandardHook()

#### Weapon Blades of Fire Condition ####

def weaponBladesOfFireOnDealingDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        #Blades of Fire Bonus Damage; Originally it was 1d6 in Complete Arcane but was raised to 1d8 in the Spell Compendium
        damageDice = dice_new('1d8')
        damageType = D20DT_FIRE
        damageMesId = 3010 #ID 3010 added in damage.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def weaponBladesOfFireGlowType(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        if not evt_obj.return_val:
            evt_obj.return_val = 6 #Fire Glow
    return 0

weaponBladesOfFire = PythonModifier("Weapon Blades of Fire", 5) # empty, empty, inventoryLocation, empty, spell_id
weaponBladesOfFire.AddHook(ET_OnDealingDamage, EK_NONE, weaponBladesOfFireOnDealingDamage, ())
weaponBladesOfFire.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponBladesOfFireGlowType, ())


#### Weapon Blades of Fire Tooltip Condition ####

weaponBladesOfFireToolTip = PythonModifier("Weapon Blades of Fire Tooltip", 5) # empty, empty, inventoryLocation, empty, spellId
weaponBladesOfFireToolTip.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.itemTooltip, ())
weaponBladesOfFireToolTip.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.itemEffectTooltip, ())

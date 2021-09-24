from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Sonic Weapon"

def sonicWeaponSpellAddWeaponCondition(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Sonic', 0, 0, 0, 0, spellId)

    #Add buff symbol to wearer
    itemWearer = attachee.obj_get_obj(obj_f_item_parent)
    itemWearer.condition_add_with_args("Weapon Sonic Buff Symbol", spellId, args.get_arg(1))
    return 0

def checkBuffSymobl(attachee, args, evt_obj):
    itemWearer = attachee.obj_get_obj(obj_f_item_parent)
    wearerHasBuffSymbol = itemWearer.d20_query("PQ_Has_Weapon_Sonic_Buff_Symbol")
    if not wearerHasBuffSymbol:
        mainhandWeapon = itemWearer.item_worn_at(item_wear_weapon_primary)
        offhandWeapon = itemWearer.item_worn_at(item_wear_weapon_secondary)
        if mainhandWeapon.item_has_condition("Weapon Sonic") or offhandWeapon.item_has_condition("Weapon Sonic"):
            itemWearer.condition_add_with_args("Weapon Sonic Buff Symbol", args.get_arg(0), args.get_arg(1))
    return 0

def sonicWeaponSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Sonic', args.get_arg(0))
    return 0

sonicWeaponSpell = PythonModifier("sp-Sonic Weapon", 3) # spell_id, duration, empty
sonicWeaponSpell.AddHook(ET_OnConditionAdd, EK_NONE, sonicWeaponSpellAddWeaponCondition,())
sonicWeaponSpell.AddHook(ET_OnConditionRemove, EK_NONE, sonicWeaponSpellWeaponConditionRemove, ())
sonicWeaponSpell.AddHook(ET_OnBeginRound, EK_NONE, checkBuffSymobl, ())
sonicWeaponSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
sonicWeaponSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
sonicWeaponSpell.AddSpellDispelCheckStandard()
sonicWeaponSpell.AddSpellTeleportPrepareStandard()
sonicWeaponSpell.AddSpellTeleportReconnectStandard()
sonicWeaponSpell.AddSpellCountdownStandardHook()

#### Weapon Sonic Condition ####

def weaponSonicOnDealingDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        damageDice = dice_new('1d6') #Sonic Weapon Bonus Damage
        damageType = D20DT_SONIC
        damageMesId = 3001 #ID 3001 added in damage.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def weaponSonicGlowType(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        if not evt_obj.return_val:
            evt_obj.return_val = 7 #there is no sonic Weapon Glow in the game I think using holy for now
    return 0

weaponSonic = PythonModifier("Weapon Sonic", 5) # empty, empty, inventoryLocation, empty, spell_id
weaponSonic.AddHook(ET_OnDealingDamage, EK_NONE, weaponSonicOnDealingDamage, ())
weaponSonic.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponSonicGlowType, ())

#### Weapon Sonic Buff Symbol Condition ####

def verifyBuffSymbol(attachee, args, evt_obj):
    mainhandWeapon = attachee.item_worn_at(item_wear_weapon_primary)
    offhandWeapon = attachee.item_worn_at(item_wear_weapon_secondary)
    if mainhandWeapon.item_has_condition("Weapon Sonic"):
        return 0
    elif offhandWeapon.item_has_condition("Weapon Sonic"):
        return 0
    args.condition_remove()
    return 0

def answerQuery(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def buffSymbolTickdown(attachee, args, evt_obj):
    args.set_arg(1, args.get_arg(1)-evt_obj.data1)
    if args.get_arg(1) < 0:
        args.condition_remove()
    return 0

weaponSonicBuffSymbol = PythonModifier("Weapon Sonic Buff Symbol", 3) # spell_id, duration, empty
weaponSonicBuffSymbol.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
weaponSonicBuffSymbol.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
weaponSonicBuffSymbol.AddHook(ET_OnD20Signal, EK_S_Inventory_Update, verifyBuffSymbol, ())
weaponSonicBuffSymbol.AddHook(ET_OnD20PythonQuery, "PQ_Has_Weapon_Sonic_Buff_Symbol", answerQuery, ())
weaponSonicBuffSymbol.AddHook(ET_OnBeginRound, EK_NONE, buffSymbolTickdown, ())

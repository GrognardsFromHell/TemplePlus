from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Lawful Sword"

def lawfulSwordSpellAddWeaponCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Lawful Sword', 0, 0, 0, 0, args.get_arg(0))
    return 0

def lawfulSwordSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Lawful Sword', args.get_arg(0))
    return 0

lawfulSwordSpell = PythonModifier("sp-Lawful Sword", 3) # spell_id, duration, empty
lawfulSwordSpell.AddHook(ET_OnConditionAdd, EK_NONE, lawfulSwordSpellAddWeaponCondition,())
lawfulSwordSpell.AddHook(ET_OnConditionRemove, EK_NONE, lawfulSwordSpellWeaponConditionRemove, ())
lawfulSwordSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
lawfulSwordSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
lawfulSwordSpell.AddSpellDispelCheckStandard()
lawfulSwordSpell.AddSpellTeleportPrepareStandard()
lawfulSwordSpell.AddSpellTeleportReconnectStandard()
lawfulSwordSpell.AddSpellCountdownStandardHook()

###### Lawful Sword Weapon Condition ######

def weaponLawfulSwordBonusToHit(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        bonusValue = 5 #Lawful Sword grants a +5 enhancement bonus to hit
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Lawful Sword~[TAG_SPELLS_LAWFUL_SWORD]")
    return 0

def weaponLawfulSwordOnDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        spellPacket = tpdp.SpellPacket(args.get_arg(0))
        target = evt_obj.attack_packet.target
        targetAlignment = target.critter_get_alignment()
        #Add enhancement bonus to damage
        bonusValue = 5 #Lawful Sword grants a +5 enhancement bonus to damage
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.damage_packet.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Lawful Sword~[TAG_SPELLS_LAWFUL_SWORD]")
        #Check if magic damage property needs to be added
        if not evt_obj.damage_packet.attack_power & D20DAP_MAGIC:
            evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
        #Check if Lawful damage property needs to be added
        if not evt_obj.damage_packet.attack_power & D20DAP_LAW:
            evt_obj.damage_packet.attack_power |= D20DAP_LAW
            game.particles('hit-LAW-medium', evt_obj.attack_packet.target)
        #Add extra damage if target is chaotic
        if targetAlignment & ALIGNMENT_CHAOTIC:
            damageDice = dice_new('1d6')
            damageDice.number = 2
            damageType = D20DT_UNSPECIFIED
            damageMesId = 3007 #ID 3007 added in damage.mes
            evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
    return 0

def weaponLawfulSwordGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        evt_obj.return_val = 8 #Law Glow effect
    return 0
weaponLawfulSword = PythonModifier("Weapon Lawful Sword", 5) # empty, empty, inventoryLocation, empty, spell_id
weaponLawfulSword.AddHook(ET_OnToHitBonus2, EK_NONE, weaponLawfulSwordBonusToHit, ())
weaponLawfulSword.AddHook(ET_OnDealingDamage, EK_NONE, weaponLawfulSwordOnDamage, ())
weaponLawfulSword.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponLawfulSwordGlowEffect, ())

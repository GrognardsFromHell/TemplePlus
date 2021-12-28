from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Spikes"

def spikesSpellAddWeaponCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Spikes', 0, args.get_arg(2), 0, 0, args.get_arg(0))
    return 0

def spikesSpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Spikes', args.get_arg(0))
    return 0

spikesSpell = PythonModifier("sp-Spikes", 4) # spell_id, duration, bonusDamage, empty
spikesSpell.AddHook(ET_OnConditionAdd, EK_NONE, spikesSpellAddWeaponCondition,())
spikesSpell.AddHook(ET_OnConditionRemove, EK_NONE, spikesSpellWeaponConditionRemove, ())
spikesSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
spikesSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
spikesSpell.AddSpellDispelCheckStandard()
spikesSpell.AddSpellTeleportPrepareStandard()
spikesSpell.AddSpellTeleportReconnectStandard()
spikesSpell.AddSpellCountdownStandardHook()

###### Spikes Weapon Condition ######

def weaponSpikesToHitBonus(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        bonusValue = 2 #Spikes grants a +2 enhancement bonus to hit
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Spikes~[TAG_SPELLS_SPIKES]")
    return 0

def weaponSpikesBonusToDamage(attachee, args, evt_obj):
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
    bonusValue = args.get_arg(1) #Spikes grants a bonus equal to the casterlevel on damage (caps at 10)
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.damage_packet.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Spikes~[TAG_SPELLS_SPIKES]")
    return 0

def weaponSpikesModifyCritRange(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        bonusValue = usedWeapon.obj_get_int(obj_f_weapon_crit_range) #Spikes doubles Threat Range
        bonusType = 12 #ID 12 = Enhancement
        evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Spikes~[TAG_SPELLS_SPIKES]")
    return 0

def weaponSpikesGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        if not evt_obj.return_val:
            evt_obj.return_val = 1
    return 0

weaponSpikes = PythonModifier("Weapon Spikes", 5) # empty, bonusDamage, inventoryLocation, empty, spell_id
weaponSpikes.AddHook(ET_OnToHitBonus2, EK_NONE, weaponSpikesToHitBonus, ())
weaponSpikes.AddHook(ET_OnDealingDamage, EK_NONE, weaponSpikesBonusToDamage, ())
weaponSpikes.AddHook(ET_OnGetCriticalHitRange, EK_NONE, weaponSpikesModifyCritRange, ())
weaponSpikes.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponSpikesGlowEffect, ())

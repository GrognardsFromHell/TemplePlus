from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Weapon of Energy"

def weaponOfEnergySpellAddWeaponCondition(attachee, args, evt_obj):
    spellPacket = tpdp.SpellPacket(args.get_arg(0))
    spellPacket.add_target(attachee, 0)
    attachee.item_condition_add_with_args('Weapon Energy', 0, args.get_arg(2), 0, 0, args.get_arg(0))
    return 0

def weaponOfEnergySpellWeaponConditionRemove(attachee, args, evt_obj):
    attachee.item_condition_remove('Weapon Energy', args.get_arg(0))
    return 0

weaponOfEnergySpell = PythonModifier("sp-Weapon of Energy", 4) # spell_id, duration, elementType, empty
weaponOfEnergySpell.AddHook(ET_OnConditionAdd, EK_NONE, weaponOfEnergySpellAddWeaponCondition,())
weaponOfEnergySpell.AddHook(ET_OnConditionRemove, EK_NONE, weaponOfEnergySpellWeaponConditionRemove, ())
weaponOfEnergySpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
weaponOfEnergySpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
weaponOfEnergySpell.AddSpellDispelCheckStandard()
weaponOfEnergySpell.AddSpellTeleportPrepareStandard()
weaponOfEnergySpell.AddSpellTeleportReconnectStandard()
weaponOfEnergySpell.AddSpellCountdownStandardHook()

#### Weapon Energy Condition ####

def getParticleEffect(elementType):
    particleDict = {
    1:"hit-ACID-medium",
    2:"hit-COLD-medium",
    3:"hit-SHOCK-medium",
    4:"hit-FIRE-medium",
    5:"hit-ACID-heavy",
    6:"hit-COLD-Burst",
    7:"hit-SHOCK-burst",
    8:"hit-FIRE-burst"
    }
    return particleDict.get(elementType)

def weaponEnergyOnDealingDamage(attachee, args, evt_obj):
    usedWeapon = evt_obj.attack_packet.get_weapon_used()
    if spell_utils.verifyItem(usedWeapon, args):
        #Set damageType to chosen element
        elementType = args.get_arg(1)
        if elementType == 1:
            damageType = D20DT_ACID
        elif elementType == 2:
            damageType = D20DT_COLD
        elif elementType == 3:
            damageType = D20DT_ELECTRICITY
        elif elementType == 4:
            damageType = D20DT_FIRE
        else:
            return 0
        #Add damage dice
        damageDice = dice_new('1d6')
        damageDice.number = 2
        damageMesId = 3008 #ID 3008 added in damage.mes
        evt_obj.damage_packet.add_dice(damageDice, damageType, damageMesId)
        #Handle Crit
        isCritical = evt_obj.attack_packet.get_flags() & D20CAF_CRITICAL
        if isCritical:
            damageDiceBurst = dice_new('1d10')
            critMultiplier = evt_obj.damage_packet.critical_multiplier
            if critMultiplier > 3: #unsure if needed; D20SRD description ends at x4
                critMultiplier = 3
            damageDiceBurst.number = critMultiplier
            evt_obj.damage_packet.add_dice(damageDiceBurst, damageType, damageMesId)
        #Add particle effects
        particleEffect = getParticleEffect(elementType)
        game.particles("{}".format(particleEffect), evt_obj.attack_packet.target)
        if isCritical:
            elementType += 4
            particleCrit = getParticleEffect(elementType)
            game.particles("{}".format(particleCrit), evt_obj.attack_packet.target)
    return 0

def weaponEnergyGlowEffect(attachee, args, evt_obj):
    usedWeapon = evt_obj.get_obj_from_args()
    if spell_utils.verifyItem(usedWeapon, args):
        elementType = args.get_arg(1)
        if elementType == 1: #acid
            evt_obj.return_val = 2
        elif elementType == 2: #cold
            evt_obj.return_val = 5
        elif elementType == 3: #electricity
            evt_obj.return_val = 9
        elif elementType == 4: #fire
            evt_obj.return_val = 6
    return 0

weaponEnergy = PythonModifier("Weapon Energy", 5) # empty, elementType, inventoryLocation, empty, spell_id
weaponEnergy.AddHook(ET_OnDealingDamage, EK_NONE, weaponEnergyOnDealingDamage, ())
weaponEnergy.AddHook(ET_OnWeaponGlowType, EK_NONE, weaponEnergyGlowEffect, ())
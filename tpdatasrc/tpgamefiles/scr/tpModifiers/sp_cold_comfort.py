from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, getElementEnum

print "Registering sp-Cold Comfort"

coldComfortSpell = AuraSpellHandleModifier("sp-Cold Comfort") #spellId, duration, damageType, spellEventId, spellDc, empty
coldComfortSpell.AddSpellDismiss()
coldComfortSpell.AddSpellNoDuplicate()

### Start Cold Comfort Effect ###

def queryHasEndure(attachee, args, evt_obj):
    evt_obj.return_val = 1
    #sp-Endure Elements uses descriptors constants
    #I do use damage type constants directly, so I need to remap
    damageType = args.get_arg(2)
    evt_obj.data1 = getElementEnum(damageType)
    return 0

def addEnergyResistance(attachee, args, evt_obj):
    resistanceAmount = 5
    damageType = args.get_arg(2)
    damageMesId = 124 # ID124 = ~Damage Resistance~[TAG_SPECIAL_ABILITIES_RESISTANCE_TO_ENERGY]
    evt_obj.damage_packet.add_damage_resistance(bonusValue, damageType, damageMesId)
    return 0

def queryFireballOk(attachee, args, evt_obj):
    damageType = args.get_arg(2)
    if damageType == D20DT_FIRE:
        evt_obj.return_val = 1
    return 0

def addParticles(attachee, args, evt_obj):
    damageType = args.get_arg(2)
    particlesLabel = "sp-Endure Elements-cold" if damageType == D20DT_COLD else "sp-Endure Elements-fire"
    return 0

coldComfortEffect = AuraSpellEffectModifier("Cold Comfort") #spellId, duration, damageType, spellEventId, spellDc, empty
coldComfortEffect.AddHook(ET_OnTakingDamage2, EK_NONE, addEnergyResistance, ())
coldComfortEffect.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Resist_Elements, queryHasEndure, ())
coldComfortEffect.AddHook(ET_OnD20Query, EK_Q_AI_Fireball_OK, queryFireballOk, ())
#coldComfortEffect.AddHook(ET_OnConditionAdd, EK_NONE, addParticles, ())
coldComfortEffect.AddSpellDismiss()

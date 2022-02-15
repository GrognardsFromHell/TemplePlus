from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Visage of the Deity lesser"

def visageOfTheDeityLesserSpellElementalResistance(attachee, args, evt_obj):
    casterAlignment = attachee.critter_get_alignment() #Visage is a personal target only spell
    if casterAlignment & ALIGNMENT_GOOD:
        resistanceValue = 10 #Visage of the Deity, lesser grants 10 resistance to different energy types
        damageMesId = 124 #ID 124 in damage.mes is Resistance to Energy
        evt_obj.damage_packet.add_damage_resistance(resistanceValue, D20DT_ACID, damageMesId)
        evt_obj.damage_packet.add_damage_resistance(resistanceValue, D20DT_COLD, damageMesId)
        evt_obj.damage_packet.add_damage_resistance(resistanceValue, D20DT_ELECTRICITY, damageMesId)
    elif casterAlignment & ALIGNMENT_EVIL:
        evt_obj.damage_packet.add_damage_resistance(resistanceValue, D20DT_COLD, damageMesId)
        evt_obj.damage_packet.add_damage_resistance(resistanceValue, D20DT_FIRE, damageMesId)
    return 0

visageOfTheDeityLesserSpell = SpellPythonModifier("sp-Visage of the Deity lesser") # spell_id, duration, empty
visageOfTheDeityLesserSpell.AddHook(ET_OnTakingDamage2, EK_NONE, visageOfTheDeityLesserSpellElementalResistance,())
visageOfTheDeityLesserSpell.AddAbilityBonus(4, bonus_type_enhancement, stat_charisma)
visageOfTheDeityLesserSpell.AddSpellNoDuplicate()

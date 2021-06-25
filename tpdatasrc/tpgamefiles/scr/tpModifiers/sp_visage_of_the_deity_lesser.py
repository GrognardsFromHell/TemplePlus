from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Visage of the Deity lesser"

def visageOfTheDeityLesserSpellAbilityBonus(attachee, args, evt_obj):
    bonusValue = 4 #Visage of the Deity, lesser adds a +4 Enhancement Bonus to Charisma
    bonusType = 12 #ID 12 = Enhancement
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Enhancement~[TAG_ENHANCEMENT_BONUS] : ~Visage of the Deity, lesser~[TAG_SPELLS_VISAGE_OF_THE_DEITY_LESSER]")
    return 0

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

visageOfTheDeityLesserSpell = PythonModifier("sp-Visage of the Deity lesser", 3, False) # spell_id, duration, empty
visageOfTheDeityLesserSpell.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CHARISMA, visageOfTheDeityLesserSpellAbilityBonus, ())
visageOfTheDeityLesserSpell.AddHook(ET_OnTakingDamage , EK_NONE, visageOfTheDeityLesserSpellElementalResistance,())
visageOfTheDeityLesserSpell.AddHook(ET_OnConditionAddPre, EK_NONE, spell_utils.replaceCondition, ())
visageOfTheDeityLesserSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
visageOfTheDeityLesserSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
visageOfTheDeityLesserSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
visageOfTheDeityLesserSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
visageOfTheDeityLesserSpell.AddSpellDispelCheckStandard()
visageOfTheDeityLesserSpell.AddSpellTeleportPrepareStandard()
visageOfTheDeityLesserSpell.AddSpellTeleportReconnectStandard()
visageOfTheDeityLesserSpell.AddSpellCountdownStandardHook()

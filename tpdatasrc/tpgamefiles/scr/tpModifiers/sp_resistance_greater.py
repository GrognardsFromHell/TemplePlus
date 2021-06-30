from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Resistance Greater"

def resistanceGreaterSpellBonus(attachee, args, evt_obj):
    bonusValue = 3 #Resistance, Greater is a flat +3 resistance bouns to saves
    bonusType = 15 #ID 15 = Resistance Bonus
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Resistance, Greater~[TAG_SPELLS_RESISTANCE_GREATER] ~Resistance~[TAG_MODIFIER_RESISTANCE] Bonus")
    return 0

resistanceGreaterSpell = PythonModifier("sp-Resistance Greater", 3, False) # spell_id, duration, empty
resistanceGreaterSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, resistanceGreaterSpellBonus,())
resistanceGreaterSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
resistanceGreaterSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
resistanceGreaterSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
resistanceGreaterSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
resistanceGreaterSpell.AddSpellDispelCheckStandard()
resistanceGreaterSpell.AddSpellTeleportPrepareStandard()
resistanceGreaterSpell.AddSpellTeleportReconnectStandard()
resistanceGreaterSpell.AddSpellCountdownStandardHook()

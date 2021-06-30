from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Divine Protection"

def divineProtectionSpellBonus(attachee, args, evt_obj):
    bonusValue = 1 #Divine Protection grants a +1 morale bonus to AC and saving throws
    bonusType = 13 #ID 13 = Morale
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Morale~[TAG_MODIFIER_MORALE] : ~Divine Protection~[TAG_SPELLS_DIVINE_PROTECTION]")
    return 0

divineProtectionSpell = PythonModifier("sp-Divine Protection", 3, False) # spell_id, duration, empty
divineProtectionSpell.AddHook(ET_OnGetAC, EK_NONE, divineProtectionSpellBonus,())
divineProtectionSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, divineProtectionSpellBonus,())
divineProtectionSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, ())
divineProtectionSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, ())
divineProtectionSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
divineProtectionSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
divineProtectionSpell.AddSpellDispelCheckStandard()
divineProtectionSpell.AddSpellTeleportPrepareStandard()
divineProtectionSpell.AddSpellTeleportReconnectStandard()
divineProtectionSpell.AddSpellCountdownStandardHook()

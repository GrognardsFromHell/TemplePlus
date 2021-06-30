from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import spell_utils
print "Registering sp-Conviction"

def convictionSpellBonusToSaves(attachee, args, evt_obj):
    bonusValue = args.get_arg(2) #Bonus is passed by spell
    bonusType = 13 #ID 13 = Morale
    evt_obj.bonus_list.add(bonusValue, bonusType, "~Morale~[TAG_MODIFIER_MORALE] : ~Conviction~[TAG_SPELLS_CONVICTION]")
    return 0

convictionSpell = PythonModifier("sp-Conviction", 4, False) # spell_id, duration, bonusValue, empty
convictionSpell.AddHook(ET_OnSaveThrowLevel, EK_NONE, convictionSpellBonusToSaves, ())
convictionSpell.AddHook(ET_OnGetTooltip, EK_NONE, spell_utils.spellTooltip, (spell_conviction,))
convictionSpell.AddHook(ET_OnGetEffectTooltip, EK_NONE, spell_utils.spellEffectTooltip, (spell_conviction,))
convictionSpell.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, spell_utils.queryActiveSpell, ())
convictionSpell.AddHook(ET_OnD20Signal, EK_S_Killed, spell_utils.spellKilled, ())
convictionSpell.AddSpellDispelCheckStandard()
convictionSpell.AddSpellTeleportPrepareStandard()
convictionSpell.AddSpellTeleportReconnectStandard()
convictionSpell.AddSpellCountdownStandardHook()

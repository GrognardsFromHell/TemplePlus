from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, applyBonus

print "Registering sp-Harmonic Chorus"

def harmonicChorusSpellBonusToCasterLevel(attachee, args, evt_obj):
    evt_obj.return_val += 2
    return 0

#Harmonic Chorus requires concentration and therefor I will skip
#Duplicate check. This means that you can't add a second Harmonic
#Chorus from a different Bard (which would not stack anyways)
#To prolong the duration of the spell , but this looks like an
#Unlikely edge case to me.
harmonicChorusSpell = SpellPythonModifier("sp-Harmonic Chorus") # spell_id, duration, empty
harmonicChorusSpell.AddHook(ET_OnGetCasterLevelMod, EK_NONE, harmonicChorusSpellBonusToCasterLevel, ())
harmonicChorusSpell.AddHook(ET_OnGetSpellDcMod, EK_NONE, applyBonus, (2, bonus_type_morale,))
harmonicChorusSpell.AddSpellConcentration()

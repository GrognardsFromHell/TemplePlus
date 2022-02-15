from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Distract Assailant"

def distractAssailantSpellSetFlankedCondition(attachee, args, evt_obj):
    attachee.condition_add('Flatfooted')
    return 0

distractAssailantSpell = SpellPythonModifier("sp-Distract Assailant") # spell_id, duration, empty
distractAssailantSpell.AddHook(ET_OnConditionAdd, EK_NONE, distractAssailantSpellSetFlankedCondition,())

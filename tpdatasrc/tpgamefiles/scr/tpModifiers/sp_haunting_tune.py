from toee import *
import tpdp
from spell_utils import SpellPythonModifier

print "Registering sp-Haunting Tune"

#Helper Condition to handle Shaken

def addShakenCondition(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    duration = args.get_arg(1)
    attachee.condition_add_with_args("Shaken", spellId, duration, 0)
    return 0

def removeShakenCondition(attachee, args, evt_obj):
    duration = 0
    attachee.d20_query("sp-Remove Shaken", duration, 0, 0)
    return 0

hauntingTuneSpell = SpellPythonModifier("sp-Haunting Tune") # spell_id, duration, empty
hauntingTuneSpell.AddHook(ET_OnConditionAdd, EK_NONE, addShakenCondition,())
hauntingTuneSpell.AddHook(ET_OnConditionRemove, EK_NONE, removeShakenCondition, ())

from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Ray of Dizziness"

def rayOfDizzinessSpellTurnBasedStatusInit(attachee, args, evt_obj):
    if evt_obj.tb_status.hourglass_state > 2:
        attachee.float_text_line("Dizzy", tf_red)
        evt_obj.tb_status.hourglass_state = 2 # Limited to a Standard or Move Action only
    return 0

rayOfDizzinessSpell = SpellPythonModifier("sp-Ray of Dizziness") # spellId, duration, empty
rayOfDizzinessSpell.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, rayOfDizzinessSpellTurnBasedStatusInit, ())

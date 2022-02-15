from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Lightfoot"

def lightfootSpellCancelAoO(attachee, args, evt_obj):
    attachee.float_text_line("Lightfooted")
    evt_obj.return_val = 0
    return 0

lightfootSpell = SpellPythonModifier("sp-Lightfoot") # spell_id, duration, empty
lightfootSpell.AddHook(ET_OnD20Query, EK_Q_AOOIncurs, lightfootSpellCancelAoO,())

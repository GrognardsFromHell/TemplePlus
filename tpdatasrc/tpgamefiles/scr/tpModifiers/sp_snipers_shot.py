from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Snipers Shot"

def enableSneakAttack(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

snipersShotSpell = SpellPythonModifier("sp-Snipers Shot") #spell_id, duration, empty
snipersShotSpell.AddHook(ET_OnD20PythonQuery, "Disable Sneak Attack Range Requirement", enableSneakAttack,())

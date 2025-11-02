
from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import countAfterConcentration

print "registering sp-Rage extender"

def NoOp(critter, args, evt_obj):
	return 0

rage = PythonModifier()
rage.ExtendExisting('sp-Rage')
rage.ReplaceHook(14, ET_OnBeginRound, EK_NONE, countAfterConcentration, ())
rage.ReplaceHook(15, ET_OnBeginRound, EK_NONE, NoOp, ())

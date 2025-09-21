from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def NoAoO(critter, args, evt_obj):
	evt_obj.return_val = 0

	return 0

calm = PythonModifier()
calm.ExtendExisting('sp-Calm Emotions')
calm.AddHook(ET_OnD20Query, EK_Q_AOOPossible, NoAoO, ())

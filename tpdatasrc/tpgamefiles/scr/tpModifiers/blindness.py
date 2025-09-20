from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering blindness extensions"

def NoRun(critter, args, evt_obj):
	action = evt_obj.data1

	if action == D20A_RUN or action == D20A_CHARGE:
		if critter.d20_query('Blindsight Range') <= 0:
			evt_obj.return_val = AEC_INVALID_ACTION

	return 0

blindness = PythonModifier()
blindness.ExtendExisting('Blindness')
blindness.AddHook(ET_OnD20Query, EK_Q_ActionAllowed, NoRun, ())

sp_blindness = PythonModifier()
sp_blindness.ExtendExisting('sp-Blindness')
sp_blindness.AddHook(ET_OnD20Query, EK_Q_ActionAllowed, NoRun, ())

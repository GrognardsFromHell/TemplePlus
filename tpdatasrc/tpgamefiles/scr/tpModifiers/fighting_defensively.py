from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Fighting Defensively addendum"

def IsFightingDefensively(attachee, args, evt_obj):
	if args.get_arg(0) != 0 and args.get_arg(1) != 0:
		evt_obj.return_val = 1
		return 0
	return 0


modExtender = PythonModifier()
modExtender.ExtendExisting("Fighting Defensively")
modExtender.AddHook(ET_OnD20Query, EK_Q_FightingDefensively, IsFightingDefensively, ())
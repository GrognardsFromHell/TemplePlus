from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Opportunist extender"

def OpportunistReset(attachee, args, evt_obj):
	args.set_arg(0, 1)
	return 0


modExtender = PythonModifier()
modExtender.ExtendExisting("Opportunist")
modExtender.AddHook(ET_OnBeginRound, EK_NONE, OpportunistReset, ())
modExtender.MapToFeat(feat_opportunist)
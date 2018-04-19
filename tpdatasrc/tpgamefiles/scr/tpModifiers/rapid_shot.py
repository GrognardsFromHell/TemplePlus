from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Rapid Shot extender"

def RapidShotEnabled(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0
	
rapidShotExtender = PythonModifier()
rapidShotExtender.ExtendExisting("Rapid_Shot")
rapidShotExtender.MapToFeat(feat_rapid_shot)
rapidShotExtender.AddHook(ET_OnD20PythonQuery, "Rapid Shot Enabled", RapidShotEnabled, ())







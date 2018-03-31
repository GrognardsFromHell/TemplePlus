from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Rapid Shot Ranger extender"

def RapidShotEnabled(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(0)
	return 0
	
rapidShotRangerExtender = PythonModifier()
rapidShotRangerExtender.ExtendExisting("Rapid_Shot_Ranger")
rapidShotRangerExtender.AddHook(ET_OnD20PythonQuery, "Rapid Shot Ranger Enabled", RapidShotEnabled, ())







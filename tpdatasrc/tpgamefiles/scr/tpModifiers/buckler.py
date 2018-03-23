from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Buckler extender"

def BucklerACBonus(attachee, args, evt_obj):
	evt_obj.return_val = args.get_arg(1)
	return 0
	
bucklerExtender = PythonModifier()
bucklerExtender.ExtendExisting("Buckler")
bucklerExtender.AddHook(ET_OnD20PythonQuery, "Buckler Bonus Disabled", BucklerACBonus, ())




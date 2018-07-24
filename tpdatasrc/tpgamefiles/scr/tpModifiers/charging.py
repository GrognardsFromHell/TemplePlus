from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Charging Extender"

def ChargingQuery(attachee, args, evt_obj):
	
	#Always return true since the condition is set
	evt_obj.return_val = 1
	
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Charging")
modExtender.AddHook(ET_OnD20PythonQuery, "Charging", ChargingQuery, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Shield Bashing"

def ShieldBashingQuery(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

shieldBashingEnch = PythonModifier("Shield Bashing", 3) # spare, spare, inv_idx
shieldBashingEnch.AddHook(ET_OnD20PythonQuery, "Has Bashing", ShieldBashingQuery, ())

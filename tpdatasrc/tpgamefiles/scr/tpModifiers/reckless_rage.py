#Reckless Rage:   Races of Stone, p. 143

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Reckless Rage"

def RecklessRageAbilityBonus(attachee, args, evt_obj):
	evt_obj.return_val = 2 #+2 to con and str (used by the C++ side)
	return 0
	
def RecklessRageACPenalty(attachee, args, evt_obj):
	evt_obj.return_val = 2 #-2 to AC (Value returned must be positive, it will be negated by the C++ side)
	return 0

recklessRage = PythonModifier("Reckless Rage", 2) # args are just-in-case placeholders
recklessRage.MapToFeat("Reckless Rage")
recklessRage.AddHook(ET_OnD20PythonQuery, "Additional Rage Stat Bonus", RecklessRageAbilityBonus, ())
recklessRage.AddHook(ET_OnD20PythonQuery, "Additional Rage AC Penalty", RecklessRageACPenalty, ())

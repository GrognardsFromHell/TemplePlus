from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Combat Casting extender"

def CombatCastingBonus(attachee, args, evt_obj):
	evt_obj.return_val += 4
	return 0

def CombatCastingUsed(attachee, args, evt_obj):
	print "Removing"
	args.condition_remove()
	print "Removing - Done"
	return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Combat_Casting")
modExtender.AddHook(ET_OnD20PythonQuery,"Combat Casting Bonus", CombatCastingBonus, ())
modExtender.AddHook(ET_OnD20PythonSignal, "Combat Casting Used", CombatCastingUsed, ())




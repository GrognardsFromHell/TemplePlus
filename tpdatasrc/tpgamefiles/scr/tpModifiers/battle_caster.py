#Battle Caster  Complete Arcane, p. 75

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Battle Caster"

#Query is to be made from any class that allows a caster to wear some armor without arcane failure
def ImprovedArcaneFailure(attachee, args, evt_obj):
	evt_obj.return_val = 1 #Return 1 to improve the class's arcane failure resistance for armor
	return 0

battleCaster = PythonModifier("Battle Caster", 2) # args are just-in-case placeholders
battleCaster.MapToFeat("Battle Caster")
battleCaster.AddHook(ET_OnD20PythonQuery, "Improved Armored Casting", ImprovedArcaneFailure, ())

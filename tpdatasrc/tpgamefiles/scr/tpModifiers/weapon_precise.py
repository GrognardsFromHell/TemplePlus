from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 7

print "Adding Precise Weapon"

def PreciseWeaponQuery(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

weaponPrecise = PythonModifier("Weapon Precise", 3) # spare, spare, inv_idx
weaponPrecise.AddHook(ET_OnD20PythonQuery, "No Shot into Melee Penalty", PreciseWeaponQuery, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Magic Item Compendium, p. 11

print "Registering Armor Freedom"

def FreeMovementQuery(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

armorFreedom = PythonModifier("Armor Freedom", 3) # spare, spare, inv_idx
armorFreedom.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Freedom_of_Movement, FreeMovementQuery, ())



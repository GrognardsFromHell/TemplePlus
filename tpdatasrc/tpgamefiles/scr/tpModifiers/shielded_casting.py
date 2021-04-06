from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Shielded Casting: Races of Stone, p. 144

print "Registering Shielded Casting"

def ShieldedCastingDisableAOO(attachee, args, evt_obj):
	#Used by the C++ side to make sure the condition does not get added multiple times
	if evt_obj.data1 != tpdp.D20ActionType.CastSpell:
		return 0
		
	#Must be wearing a shield
	item = attachee.item_worn_at(item_wear_shield)
	if item == OBJ_HANDLE_NULL:
		return 0
	
	#Buckler is not good enough
	if item.is_buckler():
		return 0
	
	evt_obj.return_val = 1 #Avoid AOO
	
	return 0

shieldedCasting = PythonModifier("Shielded Casting Feat", 2) # Spare, Spare
shieldedCasting.MapToFeat("Shielded Casting")
shieldedCasting.AddHook(ET_OnD20PythonQuery, "Disable AOO", ShieldedCastingDisableAOO, ())

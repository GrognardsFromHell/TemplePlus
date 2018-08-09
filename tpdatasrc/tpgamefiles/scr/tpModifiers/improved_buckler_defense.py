#Improved Buckler Defense:   Complete Warrior, p. 100

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Improved Buckler Defense"

#Signal disables the Buckler penalty on the C++ side if two weapon fighting
def DisableBucklerPenalty(attachee, args, evt_obj):
	mainWeapon = attachee.item_worn_at(item_wear_weapon_primary)
	secondaryWeapon = attachee.item_worn_at(item_wear_weapon_secondary)
	
	evt_obj.return_val = 0
	
	# Disable the penalty if two weapon fighting.  It could be argued that this feat should apply to two-handed 
	# weapons as well but I am interpreting it as only applying when two weapon fighting.
	if (mainWeapon != secondaryWeapon) and (mainWeapon != OBJ_HANDLE_NULL) and (secondaryWeapon != OBJ_HANDLE_NULL):
		if (mainWeapon.obj_get_int(obj_f_type) == obj_t_weapon) and (secondaryWeapon.obj_get_int(obj_f_type) == obj_t_weapon):
			evt_obj.return_val = 1 
	return 0

improvedBucklerDefense = PythonModifier("Improved Buckler Defense", 2) # args are just-in-case placeholders
improvedBucklerDefense.MapToFeat("Improved Buckler Defense")
improvedBucklerDefense.AddHook(ET_OnD20PythonQuery, "Disable Buckler Penalty", DisableBucklerPenalty, ())

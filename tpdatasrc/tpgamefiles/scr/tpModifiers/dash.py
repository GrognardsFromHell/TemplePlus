from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Dash:   Complete Warrior, p. 97

print "Registering Dash"
		
def LightArmorNoLoad(obj):
	#Light armor or no armor
	armor = obj.item_worn_at(5)
	if armor != OBJ_HANDLE_NULL:
		armorFlags = armor.obj_get_int(obj_f_armor_flags)
		if (armorFlags != ARMOR_TYPE_LIGHT) and (armorFlags !=  ARMOR_TYPE_NONE):
			return 0
			
	#No heavy or medium load
	HeavyLoad = obj.d20_query(Q_Critter_Is_Encumbered_Heavy)
	if HeavyLoad:
		return 0
	MediumLoad = obj.d20_query(Q_Critter_Is_Encumbered_Medium)
	if MediumLoad:
		return 0
		
	return 1
		
def DashOnGetBaseMoveSpeed(attachee, args, evt_obj):
	if LightArmorNoLoad(attachee):
		evt_obj.bonus_list.add(5, 0, "Dash") # Unnamed bonus
	return 0

dashModifyer = PythonModifier("Dash", 2) # args are just-in-case placeholders
dashModifyer.MapToFeat("Dash")
dashModifyer.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, DashOnGetBaseMoveSpeed, ())

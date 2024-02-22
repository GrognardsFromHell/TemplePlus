from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Agile Shield Fighter: PHB II, p. 74

def getFeatName():
	return "Agile Shield Fighter"

print "Registering {}".format(getFeatName)

def AgileShieldFighterDisableTwoWeaponFightingBonus(attachee, args, evt_obj):
	shield = attachee.item_worn_at(item_wear_shield)
	if shield == OBJ_HANDLE_NULL:
		return 0
	
	evt_obj.return_val = 1 #This feat takes over two weapon fighting
	return 0
	
def AgileShieldFighterOverrideTwoWeaponPenalty(attachee, args, evt_obj):
	shield = attachee.item_worn_at(item_wear_shield)
	if shield == OBJ_HANDLE_NULL:
		return 0

	evt_obj.return_val = 1
	return 0
	
def AgileShieldFighterGetTwoWeaponPenalty(attachee, args, evt_obj):
	shield = attachee.item_worn_at(item_wear_shield)
	if shield == OBJ_HANDLE_NULL:
		return 0
	
	evt_obj.return_val = -2
	return 0

#Need to block two weapon fighting

agileShieldFighter = PythonModifier(getFeatName(), 2) #Spare, Spare
agileShieldFighter.MapToFeat(getFeatName())
agileShieldFighter.AddHook(ET_OnD20PythonQuery, "Disable Two Weapon Fighting Bonus", AgileShieldFighterDisableTwoWeaponFightingBonus, ())
agileShieldFighter.AddHook(ET_OnD20PythonQuery, "Get Two Weapon Penalty", AgileShieldFighterGetTwoWeaponPenalty, ())
agileShieldFighter.AddHook(ET_OnD20PythonQuery, "Override Two Weapon Penalty", AgileShieldFighterOverrideTwoWeaponPenalty, ())

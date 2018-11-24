#Improved Two-Weapon Defense:   Complete Warrior, p. 100

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Improved Two-Weapon Defense"

def TwoWeaponDefenseAcBonus(attachee, args, evt_obj):
	acBonus = 2
	
	weaponPrimary = attachee.item_worn_at(item_wear_weapon_primary)
	weaponSecondary = attachee.item_worn_at(item_wear_weapon_secondary)
	
	#Weapons must not be the same
	if weaponPrimary == weaponSecondary:
		return 0
	
	#Both hands must have a weapon
	if weaponPrimary == OBJ_HANDLE_NULL or weaponSecondary == OBJ_HANDLE_NULL:
		return 0

	if attachee.d20_query(Q_FightingDefensively): # this also covers Total Defense
		acBonus = acBonus * 2
		
	evt_obj.bonus_list.add(acBonus, 29, "Improved Two-Weapon Defense")  # Shield Bonus
	return 0

improvedTwoWeaponDefense = PythonModifier("Improved Two-Weapon Defense", 2) # args are just-in-case placeholders
improvedTwoWeaponDefense.MapToFeat("Improved Two-Weapon Defense")
improvedTwoWeaponDefense.AddHook(ET_OnGetAC, EK_NONE, TwoWeaponDefenseAcBonus, ())


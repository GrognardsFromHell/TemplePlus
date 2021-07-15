from templeplus.pymod import PythonModifier
from toee import *
import tpdp

#Oversized Two-Weapon Fighting:  Complete Adventurer, p. 111

print "Registering Oversized Two-Weapon Fighting"

def OversizedTwoWeaponFightingAttackBonus(attachee, args, evt_obj):
	
	weaponPrimary = attachee.item_worn_at(item_wear_weapon_primary)
	weaponSecondary = attachee.item_worn_at(item_wear_weapon_secondary)
	
	#Only apply for full attack and checks from the character sheet
	if (evt_obj.attack_packet.get_flags() & D20CAF_FINAL_ATTACK_ROLL != 0) and (evt_obj.attack_packet.get_flags() & D20CAF_FULL_ATTACK == 0):
		return 0
	
	#Weapons must not be the same
	if weaponPrimary == weaponSecondary:
		return 0
	
	#Both hands must have a weapon
	if weaponPrimary == OBJ_HANDLE_NULL or weaponSecondary == OBJ_HANDLE_NULL:
		return 0

	#Need a medium sized weapon in the off hand
	wieldType = attachee.get_wield_type(weaponSecondary)
	
	if wieldType != 1:
		return 0
	
	#Bonus of 2 for the on and off hand will result in the same bonus as a light weapon
	evt_obj.bonus_list.add_from_feat(2, 0, 114, "Oversized Two-Weapon Fighting")
		
	return 0

oversizedTwoWeaponFighting = PythonModifier("Oversized Two-Weapon Fighting", 2) #Spare, Spare
oversizedTwoWeaponFighting.MapToFeat("Oversized Two-Weapon Fighting")
oversizedTwoWeaponFighting.AddHook(ET_OnToHitBonus2, EK_NONE, OversizedTwoWeaponFightingAttackBonus, ())


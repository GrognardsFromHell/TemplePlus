#Deadly Defense:  Complete Scoundrel, p. 76

from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Deadly Defense"

#Check if the weapons is usable with finesse 
def IsFinesseWeapon(creature, weapon):

	#Unarmed works
	if (weapon == OBJ_HANDLE_NULL):
		return 1

	#Ranged weapons don't work
	weapFlags = weapon.obj_get_int(obj_f_weapon_flags)
	if (weapFlags & OWF_RANGED_WEAPON):
		return 0
	
	#Light weapon works
	wieldType = creature.get_wield_type(weapon)
	if (wieldType == 0):
		return 1
		
	#Whip, rapier, spiked chain works
	WeaponType = weapon.get_weapon_type()
	if (WeaponType == wt_whip) or (WeaponType == wt_spike_chain) or (WeaponType == wt_rapier):
		return 1
		
	return 0

def HasLightArmorNoShield(obj):
	
	#Light armor or no armor
	armor = obj.item_worn_at(5)
	if armor != OBJ_HANDLE_NULL:
		armorFlags = armor.obj_get_int(obj_f_armor_flags)
		if (armorFlags != ARMOR_TYPE_LIGHT) and (armorFlags !=  ARMOR_TYPE_NONE):
			return 0
	
	#No Shield
	shield = obj.item_worn_at(11)
	if shield != OBJ_HANDLE_NULL:
		return 0
	return 1

def DeadlyDefenseDamageBonus(attachee, args, evt_obj):
	#Test the check box for fighting defensively only (the ability won't be active yet on the first attack)
	IsFightingDefensively = attachee.d20_query("Fighting Defensively Checked")
	
	#Combat Expertise Penalty >= 2 will also trigger the bonus
	CombatExpertiseValue = attachee.d20_query("Combat Expertise Value")
	
	if IsFightingDefensively or (CombatExpertiseValue >= 2):
	
		LightOnly = HasLightArmorNoShield(attachee)
		ValidWeapon = IsFinesseWeapon(attachee, evt_obj.attack_packet.get_weapon_used())
		
		#No armor or shield and a weapon finesse usable weapon
		if LightOnly and ValidWeapon:
			damage_dice = dice_new('1d6')
			evt_obj.damage_packet.add_dice(damage_dice, -1, 127)
	return 0

deadlyDefense = PythonModifier("Deadly Defense", 2) # args are just-in-case placeholders
deadlyDefense.MapToFeat("Deadly Defense")
deadlyDefense.AddHook(ET_OnDealingDamage, EK_NONE, DeadlyDefenseDamageBonus, ())

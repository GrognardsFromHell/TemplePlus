#Two-Weapon Rend:  Player's Handbook II, p. 84

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import math

print "Registering Two-Weapon Rend"

# List of creatures damaged by primary and secondary weapons respectively
# Rend information would be lost if the game is saved in the middle of a full attack
# but this shouldn't be a significant problem.
primaryList = list()
secondaryList = list()

def TwoWeaponRendBeginRound(attachee, args, evt_obj):
	# Clear out the list of enemies hit with the primary and secondary weapon
	primaryList[:] = []
	secondaryList[:] = []
	return 0

def TwoWeaponRendDamageBonus(attachee, args, evt_obj):
	weaponUsed = evt_obj.attack_packet.get_weapon_used()
	weaponPrimary = attachee.item_worn_at(item_wear_weapon_primary)
	weaponSecondary = attachee.item_worn_at(item_wear_weapon_secondary)
	
	#Weapons must not be the same
	if weaponPrimary == weaponSecondary:
		return 0
	
	#Both hands must have a weapon
	if weaponPrimary == OBJ_HANDLE_NULL or weaponSecondary == OBJ_HANDLE_NULL:
		return 0
	
	target = evt_obj.attack_packet.target
	bRend = false
	
	if weaponUsed == weaponPrimary:
		#Check if the target has been hit by the secondary weapon already
		if weaponUsed in secondaryList:
			#Rend only once per round.  If the target is not in the primary list then rend
			if not weaponUsed in primaryList:
				primaryList.append(target)
				bRend = true
		else:
			primaryList.append(target)
	elif weaponUsed == weaponSecondary:
		#Check if the target has been hit by the primary weapon already
		if target in primaryList:
			#Rend only once per round.  If the target is not in the secondary list then rend
			if not target in secondaryList:
				secondaryList.append(target)
				bRend = true
		else:
			secondaryList.append(target)
	
	# Note:  Damge can be applied to the primary or secondary weapon depending on the order attacks hit.
	#   This is a slight difference from the feat description but it would be a lot of work 
	#   to get the rend damage type to always match the secondary weapon and it would rarely make any
	#   difference.
	if bRend:
		diceString = '1d6'
		numDice = 1
		damage_dice = dice_new(diceString)
		damage_dice.number = numDice
		
		#Add 1 and a half times the strength score as the bonus
		strScore = attachee.stat_level_get(stat_strength)
		strMod = (strScore - 10)/2
		damage_dice.bonus = strMod + strMod/2
		evt_obj.damage_packet.add_dice(damage_dice, -1, 127)
		attachee.float_text_line("Rend!")
	return 0

twoWeaponRend = PythonModifier("Two-Weapon Rend", 2) # spare, spare
twoWeaponRend.MapToFeat("Two-Weapon Rend")
twoWeaponRend.AddHook(ET_OnDealingDamage, EK_NONE, TwoWeaponRendDamageBonus, ())
twoWeaponRend.AddHook(ET_OnBeginRound, EK_NONE, TwoWeaponRendBeginRound, ())

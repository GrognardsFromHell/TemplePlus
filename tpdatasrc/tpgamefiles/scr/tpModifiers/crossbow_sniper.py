from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Crossbow Sniper Player's Handbook II, p. 77

print "Registering Crossbow Sniper"

def CrossbowSniperFeatActive(attachee, wpn):
	if wpn == OBJ_HANDLE_NULL:
		return 0
		
	usedWeapon = wpn.get_weapon_type()
	
	#Must be using a crossbow and the the appropriate weapon focus feat to get the bonus
	if usedWeapon == wt_heavy_crossbow:
		if attachee.has_feat(feat_weapon_focus_heavy_crossbow):
			return 1
		
	if usedWeapon == wt_light_crossbow:
		if attachee.has_feat(feat_weapon_focus_light_crossbow):
			return 1
		
	if usedWeapon == wt_hand_crossbow:
		if attachee.has_feat(feat_weapon_focus_hand_crossbow):
			return 1
	return 0

def CrossbowSniperDamageBonus(attachee, args, evt_obj):
	wpn = evt_obj.attack_packet.get_weapon_used()
	active = CrossbowSniperFeatActive(attachee, wpn)
	if active:
		bon_val = (attachee.stat_level_get(stat_dexterity) - 10) / 4 #Half the dex bonus
		evt_obj.damage_packet.bonus_list.add_from_feat(bon_val, 0, 114, "Crossbow Sniper")
	return 0
	
def CrossbowSniperSneakAttackRangeIncrease(attachee, args, evt_obj):
	wpn = attachee.item_worn_at(3)
	active = CrossbowSniperFeatActive(attachee, wpn)
	if active:
		evt_obj.return_val = evt_obj.return_val + 30
	return 0
	
def CrossbowSniperSkirmishRangeIncrease(attachee, args, evt_obj):
	wpn = attachee.item_worn_at(3)
	active = CrossbowSniperFeatActive(attachee, wpn)
	if active:
		evt_obj.return_val = evt_obj.return_val + 30
	return 0

CrossbowSniper = PythonModifier("Crossbow Sniper", 2) #Spare, Spare
CrossbowSniper.MapToFeat("Crossbow Sniper")
CrossbowSniper.AddHook(ET_OnDealingDamage, EK_NONE, CrossbowSniperDamageBonus, ())
CrossbowSniper.AddHook(ET_OnD20PythonQuery, "Sneak Attack Range Increase", CrossbowSniperSneakAttackRangeIncrease, ())
CrossbowSniper.AddHook(ET_OnD20PythonQuery, "Skirmish Range Increase", CrossbowSniperSkirmishRangeIncrease, ())


from templeplus.pymod import PythonModifier
from __main__ import game
from toee import *
import tpdp

print "Registering Power Critical"

def PowerCriticalBonus(attachee, args, evt_obj):
	featWeapon = args.get_param(0)
	weaponObj = evt_obj.attack_packet.get_weapon_used()
	
	#Get the weapon type or set to the appropriate unarmed weapon type
	if weaponObj != OBJ_HANDLE_NULL:
		usedWeapon = weaponObj.get_weapon_type()
	else:
		size = attachee.obj_get_int(obj_f_size)
		if size == STAT_SIZE_SMALL:
			usedWeapon = wt_unarmed_strike_small_being
		else:
			usedWeapon = wt_unarmed_strike_medium_sized_being
	
	if featWeapon == usedWeapon:
		evt_obj.bonus_list.add_from_feat(4, 0, 114, "Power Critical")

	return 0
	
powerCriticalList = list()
	
for weapon in range(wt_gauntlet, wt_mindblade):
	featEnum = game.get_feat_for_weapon_type(weapon, "Power Critical")
	
	#Some weapons don't have the power critical feat, skip them
	if featEnum != feat_none:
		featName = game.get_feat_name(featEnum)
		powerCriticalList.append(PythonModifier(featName, 3)) #Weapon, Spare, Spare
		powerCriticalList[-1].MapToFeat(featName)
		powerCriticalList[-1].AddHook(ET_OnConfirmCriticalBonus, EK_NONE, PowerCriticalBonus, (weapon,))

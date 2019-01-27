from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Strongheart"


raceEnum = race_halfling + (3 << 5)
raceSpecModule = __import__('race102_strongheart_halfling')
###################################################


def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_rogue:
		evt_obj.return_val = 1
	return 0

def HalflingFearSaveBonus(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & (1 << (D20STD_F_SPELL_DESCRIPTOR_FEAR-1))): 
		evt_obj.bonus_list.add(2, 13, 139)
	return 0
	
#+1 with thrown weapons and slings
def OnGetToHitBonusSlingsThrownWeapons(attachee, args, evt_obj):
	thrownWeapon = evt_obj.attack_packet.get_flags() & D20CAF_THROWN
	
	isSling = 0
	wpn = evt_obj.attack_packet.get_weapon_used()
	if wpn != OBJ_HANDLE_NULL:
		weaponType = wpn.get_weapon_type()
		if weaponType == wt_sling:
			isSling = 1
		
	#Check for sling or thrown weapon
	if thrownWeapon or isSling:
		evt_obj.bonus_list.add(1, 0, 139)
		
	return 0

#Note:  Adding the size +4 bonus to hide as a racial bonus since setting size to small does not grant the bonus
raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 2, skill_move_silently: 2, skill_climb:2, skill_jump:2, skill_hide:4})
race_utils.AddBaseMoveSpeed(raceSpecObj, 20)

raceSpecObj.AddHook(ET_OnD20Query, EK_Q_FavoredClass, OnGetFavoredClass, ())
raceSpecObj.AddHook(ET_OnSaveThrowLevel, EK_NONE, HalflingFearSaveBonus, ())
raceSpecObj.AddHook(ET_OnToHitBonus2, EK_NONE, OnGetToHitBonusSlingsThrownWeapons, ())


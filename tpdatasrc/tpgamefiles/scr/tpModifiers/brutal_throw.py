#Brutal Throw:   Complete Adventurer, p. 106

from templeplus.pymod import PythonModifier
from toee import *
import math

print "Registering Brutal Throw"

#Check if the weapon can only be thrown
def IsThrownOnly(wpnType):
	if (wpnType == wt_javelin) or (wpnType == wt_dart) or (wpnType == wt_shuriken) or (wpnType == wt_net) or (wpnType == wt_grenade):
		return 1
	return 0

def BrutalThrowAttackBonus(attachee, args, evt_obj):

	#Always add the bonus for thrown only weapons.  Also add the bonus if the flag is set (for weapons like dagger)
	wpn = evt_obj.attack_packet.get_weapon_used()
	wpnType = wpn.get_weapon_type()
	if not IsThrownOnly(wpnType):
		if evt_obj.attack_packet.get_flags() & D20CAF_THROWN == 0:
			return 0
		
	#Add differece between strength and dex bonus
	strScore = attachee.stat_level_get(stat_strength)
	strMod = math.floor((strScore - 10)/2)
	
	dexScore = attachee.stat_level_get(stat_dexterity)
	dexMod = math.floor((dexScore - 10)/2)
	
	bonus = int(strMod - dexMod) 
	
	if bonus > 0:
		evt_obj.bonus_list.add_from_feat(bonus, 0, 114, "Brutal Throw")
	
	return 0

brutalThrow = PythonModifier("Brutal Throw", 2) # args are just-in-case placeholders
brutalThrow.MapToFeat("Brutal Throw")
brutalThrow.AddHook(ET_OnToHitBonus2, EK_NONE, BrutalThrowAttackBonus, ())


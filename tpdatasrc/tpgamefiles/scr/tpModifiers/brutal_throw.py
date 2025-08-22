#Brutal Throw:   Complete Adventurer, p. 106

from templeplus.pymod import PythonModifier
from toee import *
import math

print "Registering Brutal Throw"

def BrutalThrowAttackBonus(attachee, args, evt_obj):

	#Always add the bonus for thrown only weapons.  Also add the bonus if the thrown flag is set (for weapons like dagger)
	wpn = evt_obj.attack_packet.get_weapon_used()
	
	if wpn == OBJ_HANDLE_NULL:
		return 0
	
	if not wpn.is_thrown_only_weapon():
		if evt_obj.attack_packet.get_flags() & D20CAF_THROWN == 0:
			return 0
		
	#Add differece between strength and dex bonus
	strScore = attachee.stat_level_get(stat_strength)
	strMod = (strScore - 10)/2
	
	dexScore = attachee.stat_level_get(stat_dexterity)
	dexMod = (dexScore - 10)/2
	
	bonus = int(strMod - dexMod) 
	
	if bonus > 0:
		#Bonus changed from 0 to 180 by Sagenlicht to avoid stacking
		#with other dex replacing ranged modifiers like Zen Archery
		evt_obj.bonus_list.add_from_feat(bonus, 180, 114, "Brutal Throw")
	
	return 0

brutalThrow = PythonModifier("Brutal Throw", 2) # args are just-in-case placeholders
brutalThrow.MapToFeat("Brutal Throw")
brutalThrow.AddHook(ET_OnToHitBonus2, EK_NONE, BrutalThrowAttackBonus, ())


#Brutal Throw:   Complete Adventurer, p. 106

from templeplus.pymod import PythonModifier
from toee import *
import math

print "Registering Brutal Throw"

def BrutalThrowAttackBonus(attachee, args, evt_obj):
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


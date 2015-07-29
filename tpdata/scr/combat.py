from toee import *
from utilities import *

## these functions get called from TemplePlus

# checks if target is warded from melee damage
def IsWarded( obj ):
	if (obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) != 0 or obj.d20_query_has_spell_condition(sp_Meld_Into_Stone) != 0):
		return 1
	return 0



def IsSpiritualWeapon( obj):
	if (obj.d20_query_with_data(Q_Critter_Has_Spell_Active, spell_spiritual_weapon, 1) != 0):
		return 1
	return 0
	
def GetPowerLevel( obj):
	lvl = obj.hit_dice_num
	crAdj = obj.obj_get_int(obj_f_npc_challenge_rating)
	objHpMax = obj.stat_level_get(stat_hp_max)
	if ( objHpMax <= 6):
		crAdj -= 3
	elif ( objHpMax <= 10):
		crAdj -= 2
	elif (objHpMax <= 15):
		crAdj -= 1
	return (crAdj + lvl)
	
	
	
def ShouldIgnoreTarget( attachee, target):
	if ( IsSpiritualWeapon(target)):
		return 1
	
	isIntelligent =  (attachee.stat_level_get(stat_intelligence) >= 3)
	if (not isIntelligent):
		return 0
	
	if (IsWarded(target)):
		return 1
	if (target.d20_query_has_spell_condition(sp_Summoned) != 0):
		attacheePowerLvl = GetPowerLevel(attachee)
		targetPowerLvl = GetPowerLevel(target)
		if (targetPowerLvl <= attacheePowerLvl - 3):
			return 1
	return 0
	
# return closest target but in a less dumb way than ToEE
# unused for now because I need to figure out how to return objhandles to the C++ :(
def TargetClosest( attachee, target = OBJ_HANDLE_NULL ):
	closest_jones = OBJ_HANDLE_NULL
	closest_dist = 10000000
	isIntelligent =  (attachee.stat_level_get(stat_intelligence) >= 3)
	
	for obj in game.obj_list_vicinity(attachee.location, OLC_PC | OLC_NPC):
		#print 'current closest is: ' + str(closest_jones) + '  trying ' + str(obj)
		if ( (not obj.is_friendly(attachee) ) and (obj.is_unconscious() == 0) ):
			#print 'is not friendly and not unconscious'
			if (obj.distance_to(attachee) < closest_dist):
				#print 'is closer...'
				if (not ShouldIgnoreTarget( attahcee, obj)):
					closest_jones = obj
					closest_dist = obj.distance_to(attachee)
					#print str(obj) + '\n'
	if (closest_jones != OBJ_HANDLE_NULL):
		print 'combat.py returning ' + str(closest_jones)
		return (closest_jones,)
	return (0,)
	
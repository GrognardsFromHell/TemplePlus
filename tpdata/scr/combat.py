from toee import *
from utilities import *

## these functions get called from TemplePlus

# checks if target is warded from melee damage
def IsWarded( attachee ):
	if (attachee.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) != 0 or attachee.d20_query_has_spell_condition(sp_Meld_Into_Stone) != 0):
		return 1
	return 0

def IsSummoned( attachee):
	if (attachee.d20_query_has_spell_condition(sp_Summoned) != 0):
		return 1
	return 0

def IsSpiritualWeapon(attachee):
	if (attachee.d20_query_has_spell_condition(sp_Spiritual_Weapon) != 0):
		return 1
	return 0
	
# return closest target but in a less dumb way than ToEE
def TargetClosest( attachee ):
	closest_jones = OBJ_HANDLE_NULL
	closest_dist = 10000000
	isIntelligent =  (attachee.stat_level_get(stat_intelligence) >= 3)
	
	for obj in game.obj_list_vicinity(attachee.location, OLC_PC | OLC_NPC):
		#print 'current closest is: ' + str(closest_jones) + '  trying ' + str(obj)
		if ( (not obj.is_friendly(attachee) ) and (obj.is_unconscious() == 0) ):
			#print 'is not friendly and not unconscious'
			if (obj.distance_to(attachee) < closest_dist):
				#print 'is closer...'
				if (not IsSpiritualWeapon(obj)):
					#print 'is not spiritual weapon'
					if (not isIntelligent or not IsWarded(obj)):
						#print 'is not warded or critter is dumb...'
						if (not isIntelligent or not IsSummoned(obj)):
							#print 'is not summoned or critter is dumb... YES'
							closest_jones = obj
							closest_dist = obj.distance_to(attachee)
							#print str(obj) + '\n'
	if (closest_jones != OBJ_HANDLE_NULL):
		print 'combat.py returning ' + str(closest_jones)
		return closest_jones
	return 0
	
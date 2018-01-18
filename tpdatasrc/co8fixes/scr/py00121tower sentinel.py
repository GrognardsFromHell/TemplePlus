from toee import *
from utilities import *
from toee import anyone
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if ( anyone(triggerer.group_list(),"has_follower",8002) ):
		triggerer.begin_dialog( attachee, 70 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (not attachee.has_met(game.party[0])):
			if (is_better_to_talk(attachee,game.party[0])):
				if (not critter_is_unconscious(game.party[0])):
					if (anyone( game.party[0].group_list(), "has_follower", 8002 )):
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 70 )
					else:
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 1 )
			else: 
				for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
					if (is_safe_to_talk(attachee, obj)):
						if (anyone( obj.group_list(), "has_follower", 8002 )):
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 70 )
						else:
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 1 )
	return RUN_DEFAULT


def talk_lareth( attachee, triggerer, line):
	npc = find_npc_near(attachee,8002)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	return SKIP_DEFAULT


def run_off( attachee, triggerer ):
	# loc = location_from_axis(427,406)
	# attachee.runoff(loc)
	attachee.runoff(attachee.location-3)
	Timed_Destroy(attachee, 5000)
	return RUN_DEFAULT
	

def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 20):
			return 1
	return 0
	

def call_leader(npc, pc): 
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 1)
	return 
	

def call_leaderplease(npc, pc):
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 70)
	return 
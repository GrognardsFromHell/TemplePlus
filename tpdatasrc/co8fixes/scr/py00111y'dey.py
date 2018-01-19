from toee import *
from utilities import *
from combat_standard_routines import *
from py00439script_daemon import *

def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 210 )			## ydey in party
	elif (game.global_vars[905] == 32 and attachee.map != 5053):
		triggerer.begin_dialog( attachee, 420 )			## have attacked 3 or more farm animals with ydey in party and not in mother screngs herb shop first floor
	elif ( game.quests[31].state == qs_completed ):
		triggerer.begin_dialog( attachee, 250 )			## have completed a second trip for otis quest
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[368] == 1) or (game.global_flags[313] == 1):
		if ( attachee.reaction_get( game.party[0] ) >= 0 ):
			attachee.reaction_set( game.party[0], -20 )
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[905] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	if npc_get( attachee, 2) == 1:
		for obj in triggerer.group_list():
			if (obj.name == 8022):
				triggerer.follower_remove(obj)
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
			attachee.reaction_set( pc, 50 )
		attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	leader = attachee.leader_get()
	if (leader != OBJ_HANDLE_NULL):
		if ((attachee.map == 5062) or (attachee.map == 5066) or (attachee.map == 5067)):
			game.global_flags[204] = 1
		if ((attachee.map == 5051) and (game.global_flags[204] == 1)):
			game.global_flags[204] = 0
			game.timevent_add( leave_group, ( attachee, leader ), 10000 )
	return RUN_DEFAULT


def leave_group( attachee, triggerer ):
	leader = attachee.leader_get()
	if (attachee.map == 5051) and (leader != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 400 )
	return RUN_DEFAULT
		

def test_adding_two_followers( pc, npc ):
	if (game.global_vars[450] & 2**14 == 0) and ( ( game.global_vars[450] & (2**0) ) == 0 ):
		pc.follower_add(npc)
		if pc.follower_atmax() == 0:
			pc.follower_remove( npc )
			npc_set( npc , 1 )
		else:
			pc.follower_remove( npc )
			npc_unset( npc , 1 )
	else:
		if game.party_npc_size() <= 1: # original condition - only have 1 NPC (Otis, presumeably) (or less, just in case...)
			npc_set( npc , 1 )
		else:
			npc_unset( npc , 1 )
	return
		
def buttin( attachee, triggerer, line):
	npc = find_npc_near(attachee,8014)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,160)
	return SKIP_DEFAULT
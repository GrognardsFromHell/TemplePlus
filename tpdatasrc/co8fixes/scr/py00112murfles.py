from toee import *
from utilities import find_npc_near
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 190 )			## murfles in party
	elif (game.global_vars[906] == 32 and attachee.map != 5054):
		triggerer.begin_dialog( attachee, 240 )			## have attacked 3 or more farm animals with murfles in party and not in screng's herb shop 2nd floor
	elif ( anyone(triggerer.group_list(),"has_follower",8021) ):
		triggerer.begin_dialog( attachee, 320 )			## already have ydey in party
	elif ( game.global_flags[100] == 1 ):
		triggerer.begin_dialog( attachee, 250 )			## ydey has travelled with you
	elif (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )			## have not met
	else:
		triggerer.begin_dialog( attachee, 350 )			## have met
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	else:
		game.global_flags[368] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[368] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[906] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def buttin( attachee, triggerer, line):
	npc = find_npc_near(attachee,8021)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,230)
	return SKIP_DEFAULT


def make_hate( attachee, triggerer ):
	if ( attachee.reaction_get( triggerer ) >= 20 ):
		attachee.reaction_set( triggerer, 20 )
	return SKIP_DEFAULT
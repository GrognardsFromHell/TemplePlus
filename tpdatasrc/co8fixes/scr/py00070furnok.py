from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.leader.reputation_has(32) == 1 or game.leader.reputation_has(30) == 1 or game.leader.reputation_has(29) == 1):
		attachee.float_line(11004,triggerer)
	elif (game.global_flags[61] == 1):
		triggerer.begin_dialog( attachee, 500 )
	elif (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 300 )
	elif (game.global_flags[51] == 1):
		if (game.quests[18].state == qs_completed):
			triggerer.begin_dialog( attachee, 220 )
		else:
			triggerer.begin_dialog( attachee, 210 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[58] = 1
	attachee.float_line(12014,triggerer)
	if (game.global_flags[235] == 0):
		game.global_vars[23] = game.global_vars[23] + 1
		if (game.global_vars[23] >= 2):
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[58] = 0
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[235] = 1
	ring = attachee.item_find( 6088 )
	if (ring != OBJ_HANDLE_NULL):
		ring.item_flag_set(OIF_NO_TRANSFER)
	dagger = attachee.item_find( 4058 )
	if (dagger != OBJ_HANDLE_NULL):
		dagger.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[235] = 0
	ring = attachee.item_find( 6088 )
	if (ring != OBJ_HANDLE_NULL):
		ring.item_flag_unset(OIF_NO_TRANSFER)
	dagger = attachee.item_find( 4058 )
	if (dagger != OBJ_HANDLE_NULL):
		dagger.item_flag_unset(OIF_NO_TRANSFER)
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ((attachee.area == 2) or (attachee.area == 4)):
		game.global_flags[60] = 1
	elif ((attachee.area == 1) and (game.global_flags[60] == 1)):
		game.global_flags[60] = 0
		if (attachee.money_get() >= 200000):
			leader = attachee.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				leader.begin_dialog(attachee, 400)
	return RUN_DEFAULT
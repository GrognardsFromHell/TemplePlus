from utilities import *
from toee import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5006 or attachee.map == 5013 or attachee.map == 5014 or attachee.map == 5042):
		if (game.global_vars[510] != 2):
			if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6):
				attachee.object_flag_unset(OF_OFF)
		else:
			attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_vars[533] = game.global_vars[533] + 1
	if (game.global_vars[533] >= 90):
		game.global_vars[510] = 2
		game.global_flags[504] = 1
		game.party[0].reputation_add( 61 )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	return SKIP_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (attachee.name == 20001):
		return RUN_DEFAULT
	if (anyone( triggerer.group_list(), "has_follower", 8736 )):
		return RUN_DEFAULT
	return SKIP_DEFAULT
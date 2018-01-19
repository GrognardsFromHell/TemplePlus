from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.area != 3):
		if (attachee.leader_get() != OBJ_HANDLE_NULL):
			triggerer.begin_dialog( attachee, 90 )		## morgan in party
		if (game.global_vars[910] == 32):
			triggerer.begin_dialog( attachee, 130 )		## have attacked 3 or more farm animals with morgan in party
		elif (attachee.has_met(triggerer)):
			triggerer.begin_dialog( attachee, 80 )		## have met
		else:
			triggerer.begin_dialog( attachee, 1 )		## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[291] == 1):
		attachee.object_flag_set(OF_OFF)
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
		if (game.global_vars[910] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if (attachee.area == 3):
		obj = attachee.leader_get()
		if (obj != OBJ_HANDLE_NULL):
			obj.begin_dialog(attachee, 110)
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	attachee.runoff(attachee.location-3)
	game.global_flags[291] = 1
	return RUN_DEFAULT


def get_rep( attachee, triggerer ):
	if triggerer.reputation_has( 16 ) == 0:
		triggerer.reputation_add( 16 )
	game.global_vars[26] = game.global_vars[26] + 1
	if ( game.global_vars[26] >= 3 and triggerer.reputation_has( 17 ) == 0):
		triggerer.reputation_add( 17 )
	return RUN_DEFAULT
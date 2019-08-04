from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 100 )
	elif ((not attachee.has_met(triggerer)) or (game.global_flags[130] == 0)):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 130 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
#	if (game.global_flags[343] == 1):
#		game.global_flags[343] = 0
#		attachee.object_flag_set(OF_OFF)
#	else:
#		attachee.object_flag_unset(OF_OFF)
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


def run_off( attachee, triggerer ):
	loc = location_from_axis(440,416)
	attachee.standpoint_set( STANDPOINT_NIGHT, 258 )
	attachee.standpoint_set( STANDPOINT_DAY, 258 )
	attachee.runoff(loc)
	return RUN_DEFAULT
	

def get_rep( attachee, triggerer ):
	if triggerer.reputation_has( 16 ) == 0:
		triggerer.reputation_add( 16 )
	game.global_vars[26] = game.global_vars[26] + 1
	if ( game.global_vars[26] >= 3 and triggerer.reputation_has( 17 ) == 0):
		triggerer.reputation_add( 17 )
	return RUN_DEFAULT


def move_wicked( attachee, triggerer ):
	attachee.standpoint_set( STANDPOINT_NIGHT, 258 )
	attachee.standpoint_set( STANDPOINT_DAY, 258 )
#	game.global_flags[343] = 1
#	attachee.runoff(attachee.location-3)	
	return RUN_DEFAULT
from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if ((not attachee.has_met(triggerer)) or (game.global_flags[129] == 1)):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 200 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[342] == 1):
		attachee.object_flag_unset(OF_OFF)
	if (game.global_flags[128] == 1):
		obj = attachee.leader_get()
		if (obj != OBJ_HANDLE_NULL):
			attachee.steal_from(obj)
			obj.follower_remove( attachee )
			obj.begin_dialog( attachee, 430 )
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
		if ((game.quests[56].state == qs_unknown) and (attachee.leader_get() != OBJ_HANDLE_NULL)):
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if ((obj.item_find( 2205 )  != OBJ_HANDLE_NULL) and (obj.item_find( 4002 ) != OBJ_HANDLE_NULL)):
					obj.begin_dialog( attachee, 400 )
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def go_hideout( attachee, triggerer ):
	# move Wonnilon to his hideout
	attachee.standpoint_set( STANDPOINT_NIGHT, 254 )
	attachee.standpoint_set( STANDPOINT_DAY, 254 )
	game.global_flags[342] = 1
	if (attachee.map == 5066):
		attachee.move( location_from_axis( 418, 374 ) )
	else:
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def disappear( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (attachee.can_see(obj)):
			attachee.steal_from(obj)
	attachee.object_flag_set( OF_OFF )
	return RUN_DEFAULT
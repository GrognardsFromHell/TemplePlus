from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	mycr = find_container_near( attachee, 1053 )
	if (mycr != OBJ_HANDLE_NULL and game.global_vars[705] == 0) and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**12 == 0):
			triggerer.begin_dialog( attachee, 250 )
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[979] == 1 and attachee.map == 5001):		## turns on substitute brother smyth
		attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[979] == 1):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		if (game.global_flags[875] == 1 and game.global_flags[876] == 0 and game.quests[99].state != qs_completed and not anyone( triggerer.group_list(), "has_item", 12900 )):
			game.global_flags[876] = 1
			game.timevent_add( amii_dies, (), 140000000 )
		if (game.story_state >= 3) and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**12 == 0) and not (attachee in game.party):
			for chest in game.obj_list_vicinity(attachee.location,OLC_CONTAINER):
				if (chest.name == 1053):
					return RUN_DEFAULT
			mycr = game.obj_create( 1053, location_from_axis (572L, 438L) )
			mycr.rotation = 2.5
	return RUN_DEFAULT


def amii_dies():
	game.quests[99].state = qs_botched
	game.global_flags[862] = 1
	return RUN_DEFAULT
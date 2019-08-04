from utilities import *
from Co8 import *
from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer):
	if (game.global_flags[372] == 1):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[177] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	if (obj_percent_hp(attachee) < 75):
		found_pc = OBJ_HANDLE_NULL
		for pc in game.party:
			if pc.type == obj_t_pc:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
		if found_pc != OBJ_HANDLE_NULL:
			StopCombat(attachee, 1)
			found_pc.begin_dialog( attachee, 1 )
			game.new_sid = 0
			return SKIP_DEFAULT
			
	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[177] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if ( game.global_flags[176] == 1 ):
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
		location = location_from_axis( 560, 437 )
		attachee.runoff(location)
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	attachee.runoff(attachee.location-3)
	if ( game.global_flags[176] == 0 ):
		game.timevent_add( kill_brunk, ( attachee, ), 28800000 )
		game.global_flags[176] = 1
	return RUN_DEFAULT


def kill_brunk( attachee ):
	game.global_flags[174] = 1
	return RUN_DEFAULT
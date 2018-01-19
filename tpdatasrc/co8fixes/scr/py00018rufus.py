from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 210 )			## rufus in party
	elif (game.global_vars[912] == 32 and attachee.map != 5016 and attachee.map != 5018):
		triggerer.begin_dialog( attachee, 240 )			## have attacked 3 or more farm animals with rufus in party and not in castle main hall or upper hall
	elif (game.global_flags[835] == 1 and game.global_flags[37] == 0 and game.global_flags[842] == 1 and game.global_flags[839] == 0):
		triggerer.begin_dialog(attachee,320)			## handled tower fight diplomatically and lareth is alive and have heard about prisoner lareth and have not liberated lareth
	elif (game.party[0].reputation_has( 27 ) == 1): 
		triggerer.begin_dialog( attachee, 11002 )		## have rabble-rouser reputation - rufus won't talk to you
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if ((game.global_vars[501] >= 2 and game.quests[97].state != qs_completed and game.quests[96].state != qs_completed) or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[336] = 1
	game.global_flags[284] = 1
	if (attachee.leader_get() == OBJ_HANDLE_NULL and attachee.item_find(5009) == OBJ_HANDLE_NULL and game.global_flags[850] == 1):
		game.global_flags[850] = 0
		create_item_in_inventory( 5009, attachee )
	if (game.global_flags[233] == 0):
		game.global_vars[23] = game.global_vars[23] + 1
		if (game.global_vars[23] >= 2):
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL and attachee.item_find(5009) != OBJ_HANDLE_NULL and game.global_flags[850] == 0):
		attachee.item_find(5009).destroy()
		game.global_flags[850] = 1
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[336] = 0
	game.global_flags[284] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[912] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[233] = 1
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[233] = 0
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ( attachee.stat_level_get(stat_level_fighter) >= 8 ):
		game.leader.begin_dialog( attachee, 230 )
	return SKIP_DEFAULT
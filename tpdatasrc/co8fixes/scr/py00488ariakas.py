from toee import *
from utilities import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_vars[501] == 8):
       		triggerer.begin_dialog( attachee, 1010 )	
	elif (game.global_vars[501] == 7 and attachee.leader_get() == OBJ_HANDLE_NULL):
       		triggerer.begin_dialog( attachee, 920 )
	elif (game.global_vars[501] == 7 and attachee.leader_get() != OBJ_HANDLE_NULL):
       		triggerer.begin_dialog( attachee, 910 )
	elif (game.global_vars[501] == 1 and game.quests[97].state == qs_unknown):
       		triggerer.begin_dialog( attachee, 1 )
	elif (game.global_vars[501] >= 4 and game.global_vars[501] <= 6 and attachee.leader_get() == OBJ_HANDLE_NULL):
       		triggerer.begin_dialog( attachee, 370 )
	elif (game.global_vars[501] >= 4 and game.global_vars[501] <= 6 and attachee.leader_get() != OBJ_HANDLE_NULL):
       		triggerer.begin_dialog( attachee, 620 )
	elif (game.global_vars[501] == 2 and game.quests[97].state == qs_mentioned):
       		triggerer.begin_dialog( attachee, 890 )
	elif ((game.global_vars[501] == 2 or game.global_vars[501] == 3) and game.quests[97].state == qs_accepted):
       		triggerer.begin_dialog( attachee, 140 )
      	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[501] == 1):
		attachee.object_flag_unset(OF_OFF)
	elif (game.global_flags[504] == 1 or game.quests[97].state == qs_botched):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if should_modify_CR( attachee ):
			modify_CR( attachee, get_av_level() )
	game.global_flags[502] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[502] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[525] == 1 and game.global_flags[526] == 0):
		if (game.global_flags[527] == 1):
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (is_better_to_talk(attachee, obj)):
					game.timevent_add( wakefield_talk, ( attachee, triggerer ), 1500 )
					game.global_flags[526] = 1
		else:
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (is_better_to_talk(attachee, obj)):
					game.timevent_add( suspicious_talk, ( attachee, triggerer ), 1500 )
					game.global_flags[526] = 1
	elif (game.global_vars[501] == 3 and find_npc_near(attachee, 14496) != OBJ_HANDLE_NULL and game.global_flags[503] == 0):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_better_to_talk(attachee, obj)):
				game.timevent_add( talkie_talkie, ( attachee, triggerer ), 1500 )
				game.global_flags[503] = 1
	elif (not game.party[0].reputation_has( 52 ) and game.global_vars[505] == 2 and attachee.leader_get() == OBJ_HANDLE_NULL):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_better_to_talk(attachee, obj)):
				game.timevent_add( bad_news, ( attachee, triggerer ), 1500 )
				game.global_vars[505] = 3
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	hammer = attachee.item_find( 4079 )
	hammer.item_flag_set(OIF_NO_TRANSFER)
	armor = attachee.item_find( 6397 )
	armor.item_flag_set(OIF_NO_TRANSFER)
	boots = attachee.item_find( 6020 )
	boots.item_flag_set(OIF_NO_TRANSFER)
	gloves = attachee.item_find( 6021 )
	gloves.item_flag_set(OIF_NO_TRANSFER)
	helm = attachee.item_find( 6335 )
	helm.item_flag_set(OIF_NO_TRANSFER)
	shield = attachee.item_find( 6051 )
	shield.item_flag_set(OIF_NO_TRANSFER)
	cloak = attachee.item_find( 6233 )
	cloak.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	hammer = attachee.item_find( 4079 )
	hammer.item_flag_unset(OIF_NO_TRANSFER)
	armor = attachee.item_find( 6397 )
	armor.item_flag_unset(OIF_NO_TRANSFER)
	boots = attachee.item_find( 6020 )
	boots.item_flag_unset(OIF_NO_TRANSFER)
	gloves = attachee.item_find( 6021 )
	gloves.item_flag_unset(OIF_NO_TRANSFER)
	helm = attachee.item_find( 6335 )
	helm.item_flag_unset(OIF_NO_TRANSFER)
	shield = attachee.item_find( 6051 )
	shield.item_flag_unset(OIF_NO_TRANSFER)
	cloak = attachee.item_find( 6233 )
	cloak.item_flag_unset(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def check_back( attachee, triggerer ):
	game.timevent_add( trouble, (), 172800000 )	## 2 days
	return RUN_DEFAULT


def trouble():
	game.global_vars[501] = 3
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 25):
			return 1
	return 0


def switch_to_old_man( attachee, triggerer, line):
	npc = find_npc_near(attachee,14496)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT


def talkie_talkie( attachee, triggerer ):
	attachee.turn_towards(game.party[0])
	game.party[0].begin_dialog( attachee, 150 )
	return RUN_DEFAULT


def start_alarm( attachee, triggerer ):
	game.sound( 4131, 2 )
	return RUN_DEFAULT


def set_inside_limiter( attachee, triggerer ):
	game.timevent_add( out_of_time, ( attachee, triggerer ), 7200000 )	# 2 hours
	game.global_vars[505] = 1
	return


def out_of_time( attachee, triggerer ):
	game.global_vars[505] = 2
	return


def bad_news( attachee, triggerer ):
	attachee.turn_towards(game.party[0])
	game.party[0].begin_dialog( attachee, 930 )
	return RUN_DEFAULT


def very_bad_things( attachee, triggerer ):
	game.global_vars[510] = 2
	game.global_flags[504] = 1
	game.quests[97].state = qs_botched
	game.party[0].reputation_add( 53 )
	return


def wakefield_talk( attachee, triggerer ):
	attachee.turn_towards(game.party[0])
	game.party[0].begin_dialog( attachee, 1150 )
	return RUN_DEFAULT


def suspicious_talk( attachee, triggerer ):
	attachee.turn_towards(game.party[0])
	game.party[0].begin_dialog( attachee, 1180 )
	return RUN_DEFAULT


def hextor_movie_setup( attachee, triggerer ):
	set_hextor_slides()
	return


def set_hextor_slides():
	game.moviequeue_add(611)
	game.moviequeue_add(612)
	game.moviequeue_add(613)
	game.moviequeue_add(614)
	game.moviequeue_add(615)
	game.moviequeue_add(616)
	game.moviequeue_add(617)
	game.moviequeue_add(618)
	game.moviequeue_add(619)
	game.moviequeue_add(620)
	game.moviequeue_add(621)
	game.moviequeue_add(622)
	game.moviequeue_add(623)
	game.moviequeue_add(624)
	game.moviequeue_add(625)
	game.moviequeue_add(626)
	game.moviequeue_play()
	return RUN_DEFAULT
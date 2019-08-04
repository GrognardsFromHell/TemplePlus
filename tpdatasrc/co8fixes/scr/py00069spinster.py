from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		if ( attachee.area <= 5):
			triggerer.begin_dialog( attachee, ((attachee.area * 50) + 150 + (game.random_range(0,2)*2)) )
		else:
			triggerer.begin_dialog( attachee, 252 ) # "this place smells" used as a generic
	elif (attachee.has_met( triggerer )):
		triggerer.begin_dialog( attachee, 50 )
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
	game.global_flags[364] = 1
	attachee.float_line(12014,triggerer)
	if (game.global_flags[234] == 0):
		game.global_vars[23] = game.global_vars[23] + 1
		if (game.global_vars[23] >= 2):
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[364] = 0
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[234] = 1
	cleaver = attachee.item_find( 4213 )
	cleaver.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[234] = 0
	cleaver = attachee.item_find( 4213 )
	cleaver.item_flag_unset(OIF_NO_TRANSFER)
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	randy1 = game.random_range(1,12)
	if ((attachee.map == 5014 or attachee.map == 5061) and randy1 >= 8):
		attachee.float_line(12103,triggerer)
	elif ((attachee.map == 5087 or attachee.map == 5017) and randy1 >= 6):
		attachee.float_line(12200,triggerer)
	if ((attachee.map == 5067) and randy1 >= 9):
		attachee.float_line(12095,triggerer)
	return RUN_DEFAULT


def buttin( attachee, triggerer, line):
	npc = find_npc_near(attachee,8000)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	return SKIP_DEFAULT


def switch_to_tarah( attachee, triggerer, line):
	npc = find_npc_near(attachee,8805)
	fruella = find_npc_near(attachee,8067)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(fruella)
		fruella.turn_towards(npc)
	return SKIP_DEFAULT
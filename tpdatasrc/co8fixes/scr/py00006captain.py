from toee import *
from utilities import find_npc_near
from scripts import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (anyone( triggerer.group_list(), "has_follower", 8014 ) and game.global_flags[309] == 0):
		game.global_flags[309] = 1
		triggerer.begin_dialog( attachee, 160 )
	elif (anyone( triggerer.group_list(), "has_follower", 8000 ) and game.global_flags[308] == 0):
		game.global_flags[308] = 1
		triggerer.begin_dialog( attachee, 170 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	#if should_modify_CR( attachee ): #no longer necessary!
	#	modify_CR( attachee, get_av_level() )
	dummy = 1
	return RUN_DEFAULT


def argue( attachee, triggerer, line):
	npc = find_npc_near(attachee,14001)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,110)
	return SKIP_DEFAULT


def make_elmo_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8000)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,110)
	return SKIP_DEFAULT


def make_otis_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8014)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,110)
	return SKIP_DEFAULT
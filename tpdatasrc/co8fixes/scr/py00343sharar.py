from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	return RUN_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[993] == 2):
		attachee.object_flag_unset(OF_OFF)
	elif (game.global_vars[993] == 3):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[950] = 1
	if (game.global_flags[948] == 1 and game.global_flags[949] == 1 and game.global_flags[951] == 1 and game.global_flags[952] == 1 and game.global_flags[953] == 1 and game.global_flags[954] == 1):
		game.party[0].reputation_add(40)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[950] = 0
	game.party[0].reputation_remove(40)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (not attachee.has_wielded(4161) or not attachee.has_wielded(4245)):
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	if (not attachee.has_wielded(4161) or not attachee.has_wielded(4245)):
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not attachee.has_wielded(4161) or not attachee.has_wielded(4245)):
		attachee.item_wield_best_all()
		game.new_sid = 0
	return RUN_DEFAULT


def switch_to_tarah( attachee, triggerer, line):
	npc = find_npc_near(attachee,8805)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
	return SKIP_DEFAULT
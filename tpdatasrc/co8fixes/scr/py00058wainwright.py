from toee import *
from toee import anyone
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (anyone( triggerer.group_list(), "has_follower", 8001 )): 
		triggerer.begin_dialog( attachee, 150 )
	elif (game.global_flags[933] == 1):
		triggerer.begin_dialog( attachee, 200 )
	elif (attachee.map == 5007):
		triggerer.begin_dialog( attachee, 340 )
	elif (game.global_flags[38] == 1): 
		triggerer.begin_dialog( attachee, 200 )
	elif (game.global_flags[149] == 1): 
		triggerer.begin_dialog( attachee, 220 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
		attachee.object_flag_set(OF_OFF)
	else:
		if (game.quests[20].state == qs_completed) and (attachee.map == 5007):
			attachee.object_flag_set(OF_OFF)
			game.global_flags[933] = 1
		elif (game.quests[20].state != qs_completed) and (attachee.map == 5007):
		# contingency for turning valden back on after he hides during a fight in the inn, other stuff doesn't do it like normal
			if (not game.combat_is_active()):
				attachee.object_flag_unset(OF_OFF)
		elif (attachee.map == 5044) and (game.global_flags[933] == 1):
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	if (attachee.map != 5044):
		for pc in game.leader.group_list():
			attachee.ai_shitlist_remove(pc)
			attachee.reaction_set( pc, 80 )
	return RUN_DEFAULT


def san_exit_combat( attachee, triggerer ):
	if (attachee.map != 5044):
		attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee.map != 5044):
		attachee.object_flag_set(OF_OFF)
		game.particles( "mon-Chicken-white-hit", attachee )
		attachee.float_mesfile_line( 'mes\\float.mes', 5 )
		return SKIP_DEFAULT
	return RUN_DEFAULT


def make_hate( attachee, triggerer ):
	if ( attachee.reaction_get( triggerer ) >= 20 ):
		attachee.reaction_set( triggerer, 20 )
	return SKIP_DEFAULT



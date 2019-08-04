from toee import *
from Co8 import *
from combat_standard_routines import *
from utilities import *

def san_dialog( attachee, triggerer ):
	if (attachee.map == 5052):
		if (game.global_vars[10] == 4):
			triggerer.begin_dialog( attachee, 500 )
		else:
			triggerer.begin_dialog( attachee, 200 )
	else:
		if (game.global_vars[10] == 4):
			triggerer.begin_dialog( attachee, 500 )
		else:
			triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[97] = 1
	if (game.quests[36].state == qs_accepted):
		game.quests[36].state == qs_completed
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (game.quests[40].state != qs_accepted):
		create_item_in_inventory( 4189, attachee )
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[97] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[10] >= 2):
		game.new_sid = 0
	elif (attachee.map == 5052):
		if (not game.combat_is_active()):
			game.global_vars[31] = game.global_vars[31] + 1
			if (game.global_vars[31] > 25):
				if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
					game.global_vars[31] = 0
					n = game.random_range(190,194)
					attachee.float_line(n,triggerer)
	return RUN_DEFAULT


def brawl( attachee, triggerer ):
	attachee.reaction_set(triggerer,-100)
	game.brawl( triggerer, attachee )
	return RUN_DEFAULT


def brawl_end( attachee, triggerer, brawl_state ):
	if (brawl_state == 0):
		attachee.reaction_set(triggerer,50)
		triggerer.begin_dialog( attachee, 300 )
	elif (brawl_state == 1):
		attachee.reaction_set(triggerer,50)
		triggerer.begin_dialog( attachee, 330 )
	else:
		print "Brawl State:" + str(brawl_state)
		attachee.reaction_set(triggerer,50)
		StopCombat(attachee, 1)
		triggerer.begin_dialog( attachee, 400 )
	return RUN_DEFAULT

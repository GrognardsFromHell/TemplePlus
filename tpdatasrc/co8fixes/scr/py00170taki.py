from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 200 )			## taki in party
	elif (not attachee.has_met(triggerer)):
		if (anyone( triggerer.group_list(), "has_follower", 8040 )):
			triggerer.begin_dialog( attachee, 1 )		## have not met and ashrem is in party
		else:
			triggerer.begin_dialog( attachee, 120 )		## have not met
	elif (anyone( triggerer.group_list(), "has_follower", 8040 )):
		triggerer.begin_dialog( attachee, 170 )			## ashrem in party
	elif (game.global_vars[908] == 32):
		triggerer.begin_dialog( attachee, 250 )			## have attacked 3 or more farm animals with taki in party
	else:
		triggerer.begin_dialog( attachee, 190 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[285] == 1):
		attachee.object_flag_set(OF_OFF)
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
	attachee.float_line(12023,triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	leader = attachee.leader_get()
	if (leader != OBJ_HANDLE_NULL):
		if (triggerer.type == obj_t_npc):
			if ((triggerer.stat_level_get(stat_alignment) == LAWFUL_GOOD) or (triggerer.stat_level_get(stat_alignment) == NEUTRAL_GOOD) or (triggerer.stat_level_get(stat_alignment) == CHAOTIC_GOOD)):
				attachee.float_line(game.random_range(220,222),leader)
				return SKIP_DEFAULT
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[908] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def switch_to_ashrem( attachee, triggerer, line ):
	ashrem = find_npc_near(attachee,8040)
	if (ashrem != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(ashrem,line)
		ashrem.turn_towards(attachee)
		attachee.turn_towards(ashrem)
	return SKIP_DEFAULT


def run_off( attachee, triggerer ):
	attachee.runoff(attachee.location-3)
	game.global_flags[285] = 1
	return RUN_DEFAULT
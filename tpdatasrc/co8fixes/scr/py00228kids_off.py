from toee import *
from combat_standard_routines import *
from utilities import *

def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if (attachee.name == 8090):
		if (attachee.map == 5042):
			jaroo = find_npc_near(attachee, 20001)
			triggerer.begin_dialog(jaroo,1)
			return SKIP_DEFAULT
		elif (attachee.map == 5022 or attachee.map == 5001):
			triggerer.begin_dialog(attachee,2000)
			return SKIP_DEFAULT
	elif (attachee.name == 8068):
		if ((game.quests[106].state == qs_mentioned or game.quests[106].state == qs_completed) and game.quests[95].state != qs_completed and game.global_flags[378] == 0):
			triggerer.begin_dialog(attachee,1500)
		else:
			r = game.random_range(1,10)
			if (r == 1):
				triggerer.begin_dialog(attachee,1)
			elif (r == 2):
				triggerer.begin_dialog(attachee,10)
			elif (r == 3):
				triggerer.begin_dialog(attachee,20)
			elif (r == 4):
				triggerer.begin_dialog(attachee,30)
			elif (r == 5):
				triggerer.begin_dialog(attachee,40)
			elif (r == 6):
				triggerer.begin_dialog(attachee,50)
			elif (r == 7):
				triggerer.begin_dialog(attachee,60)
			elif (r == 8):
				triggerer.begin_dialog(attachee,70)
			elif (r == 9):
				triggerer.begin_dialog(attachee,80)
			else:
				triggerer.begin_dialog(attachee,90)
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.name == 8090):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			if (attachee.map == 5042):
				if (game.quests[99].state == qs_accepted):
					attachee.object_flag_unset(OF_OFF)
				elif (game.global_flags[862] == 1):
					attachee.object_flag_set(OF_OFF)
			elif (attachee.map == 5022):
				if (game.quests[99].state == qs_completed):
					attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.name == 14501):
		game.global_flags[862] = 1
	game.global_vars[23] = game.global_vars[23] + 1
	if (game.global_vars[23] >= 1):
		game.party[0].reputation_add( 92 )
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	if (attachee.name == 14501):
		game.global_flags[862] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
		attachee.object_flag_set(OF_OFF)
	else:
		attachee.object_flag_unset(OF_OFF)
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (obj.distance_to(attachee) <= 30 and game.global_vars[702] == 0 and critter_is_unconscious(obj) != 1):
				if (obj.stat_level_get(stat_race) == race_halfling):
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,500)
				elif (obj.stat_level_get(stat_race) == race_halforc):
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,600)
				elif (obj.stat_level_get(stat_level_paladin) >= 1):
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,200)
				elif (obj.stat_level_get(stat_level_wizard) >= 1):
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,300)
				elif (obj.stat_level_get(stat_level_bard) >= 1):
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,400)
				else:
					game.global_vars[702] = 1
					attachee.turn_towards(obj)
					obj.begin_dialog(attachee,100)
	return RUN_DEFAULT


def talk_nps( attachee, triggerer):
	npc1 = find_npc_near(triggerer,8002)
	if (npc1 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc1,5000)
		npc1.turn_towards(attachee)
	npc2 = find_npc_near(triggerer,14037)
	if (npc2 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc2,5000)
		npc2.turn_towards(triggerer)
	npc3 = find_npc_near(triggerer,8050)
	if (npc3 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc3,5000)
		npc3.turn_towards(triggerer)
	npc4 = find_npc_near(triggerer,8062)
	if (npc4 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc4,5000)
		npc4.turn_towards(triggerer)
	npc5 = find_npc_near(triggerer,8010)
	if (npc5 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc5,5000)
		npc5.turn_towards(triggerer)
	npc6 = find_npc_near(triggerer,8072)
	if (npc6 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc6,5000)
		npc6.turn_towards(triggerer)
	npc7 = find_npc_near(triggerer,8015)
	if (npc7 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc7,5000)
		npc7.turn_towards(triggerer)
	npc8 = find_npc_near(triggerer,8003)
	if (npc8 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc8,5000)
		npc8.turn_towards(triggerer)
	npc9 = find_npc_near(triggerer,8004)
	if (npc9 != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc9,5000)
		npc9.turn_towards(triggerer)
	return SKIP_DEFAULT
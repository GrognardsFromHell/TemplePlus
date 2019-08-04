from toee import *
from utilities import *
from Livonya import get_melee_reach_strategy
from py00439script_daemon import record_time_stamp, get_v, set_v, npc_set, npc_unset, npc_get, tsc, tpsts, within_rect_by_corners
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (not attachee.has_met( triggerer )):
		triggerer.begin_dialog( attachee, 1 )
	elif (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 420 )
	elif (game.global_flags[121] == 1 or game.global_flags[122] == 1 or game.global_flags[123] == 1):
		triggerer.begin_dialog( attachee, 150 )
	else:
		triggerer.begin_dialog( attachee, 160 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	# if should_modify_CR( attachee ):
		# modify_CR( attachee, get_av_level() )
	record_time_stamp(518)
	game.global_flags[110] = 1
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee.condition_add("Rend")):
		print "Added Rend to Oohlgrist"
	if (   ( obj_percent_hp(attachee)<50 ) and (game.global_flags[350] == 0) and ( get_v(454) & (2**5 + 2**7) == 0 )   ): # if he hasn't already been intimidated or regrouped
		found_pc = OBJ_HANDLE_NULL
		for pc in game.party:
			if pc.type == obj_t_pc:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
		if found_pc != OBJ_HANDLE_NULL:
			game.global_flags[349] = 1
			found_pc.begin_dialog( attachee, 70 )
			return SKIP_DEFAULT
	## THIS IS USED FOR BREAK FREE
	for obj in game.party[0].group_list():
		if (obj.distance_to(attachee) <= 9 and obj.stat_level_get(stat_hp_current) >= -9):
			return RUN_DEFAULT		
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)

	if attachee.leader_get() != OBJ_HANDLE_NULL: # Don't wanna fuck up charmed enemies
		return RUN_DEFAULT
	get_melee_reach_strategy(attachee)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[110] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			if (obj_percent_hp(attachee) > 70):
				if (group_percent_hp(leader) < 30):
					attachee.float_line(460,leader)
					attachee.attack(leader)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
##	game.global_flags[112] = 1		### removed by Livonya
	return RUN_DEFAULT


def TalkAern( attachee, triggerer, line):
	npc = find_npc_near(attachee,8033)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,480)
	return SKIP_DEFAULT


def join_temple( temple_name_input ):
	temple_name = str(temple_name_input)
	if temple_name == 'water':
		game.global_flags[112] = 1 # Oohlgrist has joined water temple
		if get_v(454) & 2**1 == 2**1: # Water has already regrouped
			set_v(454, get_v(454) | 2**6)
	elif temple_name == 'fire':
		game.global_flags[118] = 1 # Oohlgrist has joined fire temple
		if get_v(454) & 2**3 == 2**3: # Fire has already regrouped
			set_v(454, get_v(454) | 2**4)
	else:
		game.leader.damage(OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new('500d1'))
		game.leader.float_mesfile_line( 'mes\\skill_ui.mes', 155 )
	return SKIP_DEFAULT
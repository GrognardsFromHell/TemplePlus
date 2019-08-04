from toee import *
from utilities import *
from Co8 import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.quests[109].state == qs_completed):
		triggerer.begin_dialog( attachee, 430 )
	elif (game.global_flags[537] == 1):
		triggerer.begin_dialog( attachee, 400 )
	elif (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 180 )
	elif (game.global_flags[536] == 1):
		triggerer.begin_dialog( attachee, 420 )
	elif (attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 110 )
	else:
		triggerer.begin_dialog( attachee, 10 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5169 and game.global_vars[549] == 3):
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			attachee.object_flag_set(OF_OFF)
	elif (attachee.map == 5172 and game.global_vars[549] == 3):
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			attachee.object_flag_unset(OF_OFF)	
	elif (attachee.map == 5141):
		if (game.quests[109].state == qs_completed and game.global_vars[542] == 3):
			attachee.object_flag_set(OF_OFF)	
		elif (is_daytime() == 1):
			attachee.object_flag_set(OF_OFF)
		elif (is_daytime() == 0):
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	for pc in game.party:
		pc.condition_add('fallen_paladin')
	game.global_flags[539] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (triggerer.type == obj_t_pc):
		if (game.quests[109].state == qs_unknown or game.quests[109].state == qs_completed or game.quests[109].state == qs_botched):
			ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (game.global_flags[537] == 1):
		leader = game.party[0]
		StopCombat(attachee, 0)
		leader.begin_dialog( attachee, 400 )
	return SKIP_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[539] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_better_to_talk(attachee,obj)):
				game.timevent_add( howdy_ho, ( attachee, triggerer ), 2000 )
				game.new_sid = 0
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_cause_fear or spell.spell == spell_fear ):
		game.global_flags[537] = 1
		game.timevent_add( revert_ggf_537, ( attachee, triggerer ), 3600000 )
	return RUN_DEFAULT


def increment_var_536( attachee, triggerer ):
	if (not attachee.has_met(triggerer)):
		game.global_vars[536] = game.global_vars[536] + 1
	return


def increment_var_543( attachee, triggerer ):
	game.global_vars[543] = game.global_vars[543] + 1
	return


def increment_var_544( attachee, triggerer ):
	game.global_vars[544] = game.global_vars[544] + 1
	return


def increment_var_545( attachee, triggerer ):
	game.global_vars[545] = game.global_vars[545] + 1
	return


def increment_var_555( attachee, triggerer ):
	game.global_vars[555] = game.global_vars[555] + 1
	game.new_sid = 0
	return


def increment_var_556( attachee, triggerer ):
	game.global_vars[556] = game.global_vars[556] + 1
	game.new_sid = 0
	return


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 40):
			return 1
	return 0


def howdy_ho( attachee, triggerer ):
	attachee.turn_towards(game.party[0])
	game.party[0].begin_dialog( attachee, 10 )
	return RUN_DEFAULT


def gen_panathaes_loc( attachee, triggerer ):
	if (game.global_vars[539] == 0):
		chooser = game.random_range(1,8)
		if (chooser == 1):
			game.global_vars[539] = 1
		elif (chooser == 2):
			game.global_vars[539] = 2
		elif (chooser == 3):
			game.global_vars[539] = 3
		elif (chooser == 4):
			game.global_vars[539] = 4
		elif (chooser == 5):
			game.global_vars[539] = 5
		elif (chooser == 6):
			game.global_vars[539] = 6
		elif (chooser == 7):
			game.global_vars[539] = 7
		elif (chooser == 8):
			game.global_vars[539] = 8
	return


def pick_kidnapper( attachee, triggerer ):
	if (game.global_vars[542] == 0):
		picker = game.random_range(1,4)
		if (picker == 1):
			game.global_vars[542] = 1
			# boroquin is kidnapper
		elif (picker == 2 or picker == 3):
			game.global_vars[542] = 2
			# panathaes is kidnapper
		elif (picker == 4):
			game.global_vars[542] = 3
			# rakham is kidnapper
	return


def gen_kids_loc( attachee, triggerer ):
	if (game.global_vars[540] == 0 and game.global_vars[541] == 0):
		chooser = game.random_range(1,4)
		if (chooser == 1):
			game.global_vars[540] = 1
			game.global_vars[541] = 1
		elif (chooser == 2):
			game.global_vars[540] = 2
			game.global_vars[541] = 2
		elif (chooser == 3):
			game.global_vars[540] = 3
			game.global_vars[541] = 3
		elif (chooser == 4):
			game.global_vars[540] = 4
			game.global_vars[541] = 4
	return


def check_for_locket( attachee, triggerer ):
	if (game.global_vars[542] == 3):
		create_item_in_inventory(11061,attachee)
	return


def check_evidence_rep_bor( attachee, triggerer ):
	if (game.party[0].reputation_has(72)==1):
		game.party[0].reputation_add(75)
		game.party[0].reputation_remove(72)
	elif (game.party[0].reputation_has(69)==1):
		game.party[0].reputation_add(72)
		game.party[0].reputation_remove(69)
	elif (game.party[0].reputation_has(69)==0):
		if (game.party[0].reputation_has(72)==0):
			if (game.party[0].reputation_has(75)==0):
				game.party[0].reputation_add(69)
	return


def check_evidence_rep_pan( attachee, triggerer ):
	if (game.party[0].reputation_has(73)==1):
		game.party[0].reputation_add(76)
		game.party[0].reputation_remove(73)
	elif (game.party[0].reputation_has(70)==1):
		game.party[0].reputation_add(73)
		game.party[0].reputation_remove(70)
	elif (game.party[0].reputation_has(70)==0):
		if (game.party[0].reputation_has(73)==0):
			if (game.party[0].reputation_has(76)==0):
				game.party[0].reputation_add(70)
	return


def check_evidence_rep_rak( attachee, triggerer ):
	if (game.party[0].reputation_has(74)==1):
		game.party[0].reputation_add(77)
		game.party[0].reputation_remove(74)
	elif (game.party[0].reputation_has(71)==1):
		game.party[0].reputation_add(74)
		game.party[0].reputation_remove(71)
	elif (game.party[0].reputation_has(71)==0):
		if (game.party[0].reputation_has(74)==0):
			if (game.party[0].reputation_has(77)==0):
				game.party[0].reputation_add(71)
	return


def revert_ggf_537( attachee, triggerer ):
	game.global_flags[537] = 0
	return
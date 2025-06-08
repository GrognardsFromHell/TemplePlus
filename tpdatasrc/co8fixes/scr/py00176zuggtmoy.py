from utilities import *
from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_flags[181] == 1):
		game.global_flags[181] == 0
		transform_into_demon_form(attachee,triggerer,80)
	elif (triggerer.item_find(2203) != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 160 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[359] == 1):
		if (attachee.stat_level_get( stat_hp_max ) > 111):
			attachee.obj_set_int( obj_f_hp_damage, 0 )
			attachee.stat_base_set( stat_hp_max, 111 )
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (game.global_flags[183] == 1):
		game.timevent_add( zuggtmoy_banish, (attachee, triggerer), 5000 )
	else:
		game.timevent_add( zuggtmoy_die, (attachee, triggerer), 5000 )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	game.global_flags[181] = 0
	transform_into_demon_form( attachee, triggerer, -1 )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	if (obj_percent_hp(attachee) < 20):
		nearby_pc = OBJ_HANDLE_NULL
		for pc in game.party:
			if pc.type == obj_t_pc:
				attachee.ai_shitlist_remove( pc )
		for pc in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (attachee.can_see(pc)):
				if (nearby_pc == OBJ_HANDLE_NULL):
					nearby_pc = pc
				if (pc.item_find(2203)):
					pc.begin_dialog(attachee,330)
					game.new_sid = 0
					return RUN_DEFAULT
		if (nearby_pc != OBJ_HANDLE_NULL):
			nearby_pc.begin_dialog(attachee,330)
			game.new_sid = 0
			return SKIP_DEFAULT
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		nearby_unmet_pc = OBJ_HANDLE_NULL
		distant_pc = OBJ_HANDLE_NULL
		found_close_pc = 0
		for pc in game.obj_list_vicinity(attachee.location,OLC_PC):
			if ((game.global_flags[181] == 1) and (not found_close_pc)):
				if (distant_pc == OBJ_HANDLE_NULL):
					if (attachee.distance_to(pc) > 30):
						distant_pc = pc
					else:
						found_close_pc = 1
						distant_pc = OBJ_HANDLE_NULL
			if (not attachee.has_met(pc)):
				if (is_safe_to_talk(attachee,pc)):
					if (nearby_unmet_pc == OBJ_HANDLE_NULL):
						nearby_unmet_pc = pc
					if ((game.global_flags[193] == 0)) and (pc.item_find(2203) != OBJ_HANDLE_NULL):
						pc.begin_dialog(attachee,160)
						return RUN_DEFAULT
		if ((game.global_flags[193] == 0) and (nearby_unmet_pc != OBJ_HANDLE_NULL)):
			nearby_unmet_pc.begin_dialog(attachee,1)
		elif (game.global_flags[181] == 1):
			if (distant_pc != OBJ_HANDLE_NULL):
				game.global_flags[181] == 0
				transform_into_demon_form(attachee,distant_pc,80)
	return RUN_DEFAULT


def san_true_seeing( attachee, triggerer ):
	# HTN - 12/03/02, reversed flag logic, moved "transform" call and SKIP_DEFAULT inside if/else block
	# only transform once, if flag is unset
	if (game.global_flags[181] == 1):
		# set flag
		game.global_flags[181] = 0

		# perform transform and go into dialog
		transform_into_demon_form(attachee,triggerer,300)

		# SKIP ==> tells true_seeing script to put particle f/x on target (zuggtmoy)
		return SKIP_DEFAULT

	# tells true_seeing script not to put particle f/x on target (zuggtmoy)
	return RUN_DEFAULT


def transform_into_demon_form( zuggtmoy, pc, line ):
	if (game.global_flags[193] == 0):
		game.global_flags[193] = 1
		game.story_state = 6
		loc = zuggtmoy.location
		zuggtmoy.destroy()
		loc = location_from_axis( 536, 499 )
		new_zuggtmoy = game.obj_create( 14265, loc )
		if (new_zuggtmoy != OBJ_HANDLE_NULL):
			if (game.global_flags[359] == 1):
				if (new_zuggtmoy.stat_level_get( stat_hp_max ) > 111):
					new_zuggtmoy.obj_set_int( obj_f_hp_damage, 0 )
					new_zuggtmoy.stat_base_set( stat_hp_max, 111 )
			new_zuggtmoy.rotation = 3.9269908
			new_zuggtmoy.concealed_set( 1 )
			new_zuggtmoy.unconceal()
			game.particles( "mon-zug-appear", new_zuggtmoy )
			if (line != -1):
				pc.begin_dialog(new_zuggtmoy,line)
			else:
				new_zuggtmoy.attack(game.leader)
	return RUN_DEFAULT


def crone_wait( zuggtmoy, pc ):
	# turn zuggtmoy invisible
	zuggtmoy.condition_add_with_args( "Invisible", 0, 0 )
	zuggtmoy.object_flag_set(OF_DONTDRAW)
	game.global_flags[181] = 1
	return RUN_DEFAULT


def zuggtmoy_pc_persuade( zuggtmoy, pc, success, failure ):
	if (pc.saving_throw(10,D20_Save_Will,D20STD_F_NONE) == 0):
		pc.begin_dialog(zuggtmoy,failure)
	else:
		pc.begin_dialog(zuggtmoy,success)
	return SKIP_DEFAULT


def zuggtmoy_pc_charm( zuggtmoy, pc ):
	# auto dire charm the PC
	pc.dominate( zuggtmoy ) # nope, it doesn't work for adding PCs back into the party :(
	if (game.party_pc_size() == 1):
		zuggtmoy_end_game( zuggtmoy, pc )
	else:
		zuggtmoy.attack(pc)
	return SKIP_DEFAULT


def zuggtmoy_regenerate_and_attack( zuggtmoy, pc ):
#	zuggtmoy.obj_set_int( obj_f_hp_damage, 0 )
	dice = dice_new("1d10+1000")
	zuggtmoy.heal( OBJ_HANDLE_NULL, dice )
	zuggtmoy.healsubdual( OBJ_HANDLE_NULL, dice )
	zuggtmoy.attack(pc)
	return RUN_DEFAULT


def zuggtmoy_banish( zuggtmoy, pc ):
	game.global_flags[188] = 1
	game.global_flags[372] = 1
	# play banishment movie
	game.fade(0,0,301,0)
	if (game.global_flags[500] == 0):
		zuggtmoy_end_game( zuggtmoy, pc )
	elif (game.global_flags[500] == 1):
		zuggtmoy_end_game_nc( zuggtmoy, pc )
	zuggtmoy.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def zuggtmoy_die( zuggtmoy, pc ):
	game.global_flags[189] = 1
	game.global_flags[372] = 1
	# play death movie
	game.fade(0,0,302,0)
	if (game.global_flags[500] == 0):
		zuggtmoy_end_game( zuggtmoy, pc )
	elif (game.global_flags[500] == 1):
		zuggtmoy_end_game_nc( zuggtmoy, pc )
	return RUN_DEFAULT


def zuggtmoy_end_game( zuggtmoy, pc ):
	# play slides and end game
	set_end_slides( zuggtmoy, pc )
	game.moviequeue_play_end_game()
	return RUN_DEFAULT


def zuggtmoy_end_game_nc( zuggtmoy, pc ):
	# play slides and don't end game
	set_end_slides_nc( zuggtmoy, pc )
	game.moviequeue_play()
	create_item_in_inventory( 11074, game.party[0] )
	game.party[0].reputation_add(91)
	game.areas[14] = 1
	game.fade_and_teleport(0,0,0,5121,228,507)
	return RUN_DEFAULT


def zuggtmoy_pillar_gone( zuggtmoy, pc ):
	# get rid of pillar
	for obj in game.obj_list_vicinity(zuggtmoy.location,OLC_SCENERY):
		if (obj.name == 1619):
			obj.object_flag_set(OF_OFF)
			return RUN_DEFAULT
	return RUN_DEFAULT
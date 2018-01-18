from utilities import *
from combat_standard_routines import *
from toee import *


def san_dialog( attachee, triggerer ):
	if (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	elif (game.global_flags[144] == 1):
		triggerer.begin_dialog( attachee, 90 )
	else:
		triggerer.begin_dialog( attachee, 80 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer):
	if (game.global_flags[372] == 1):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[338] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	leader = game.leader
	if (not attachee.has_met(leader)) and (game.global_flags[164] == 0):	##Smigmal attacks first time party visits
		game.counters[0] = game.counters[0] + 1
		if ((game.counters[0] >= 2) and (group_percent_hp(leader) > 30) and (game.global_flags[852] == 0)):
			for pc in game.party:
				if pc.type == obj_t_pc:
					attachee.ai_shitlist_remove( pc )
			game.global_flags[852] = 1
			leader.begin_dialog( attachee, 1 )
##			game.new_sid = 0	## removed by Livonya
			return SKIP_DEFAULT
	elif (not attachee.has_met(leader)) and (game.global_flags[164] == 1):	##Smigmal attacks if Falrinth has been confronted and is away but Smigmal has not yet been met
		game.counters[0] = game.counters[0] + 1
		if ((game.counters[0] >= 1) and (group_percent_hp(leader) > 30) and (game.global_flags[996] == 0)):
			for pc in game.party:
				if pc.type == obj_t_pc:
					attachee.ai_shitlist_remove( pc )
			game.global_flags[996] = 1
			leader.begin_dialog( attachee, 130 )
##			game.new_sid = 0	## removed by Livonya
			return SKIP_DEFAULT
	elif (attachee.has_met(leader)) and (game.global_flags[167] == 1):	##Smigmal attacks if she has been met, then Falrinth has been confronted and left, and now both are back
		game.counters[0] = game.counters[0] + 1
		if ((game.counters[0] >= 4) and (group_percent_hp(leader) > 30) and (game.global_flags[996] == 0)):
			for pc in game.party:
				if pc.type == obj_t_pc:
					attachee.ai_shitlist_remove( pc )
			game.global_flags[996] = 1
			leader.begin_dialog( attachee, 120 )
##			game.new_sid = 0	## removed by Livonya
			return SKIP_DEFAULT

	## THIS IS USED FOR BREAK FREE
	for obj in game.party[0].group_list():
		if (obj.distance_to(attachee) <= 3 and obj.stat_level_get(stat_hp_current) >= -9):
			return RUN_DEFAULT		
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
##		attachee.d20_send_signal(S_BreakFree)

	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[338] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_flags[144] == 1):
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (attachee.has_met(obj)):
					if (is_safe_to_talk(attachee,obj)):
						obj.begin_dialog(attachee,90)
						game.new_sid = 0
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (triggerer.type == obj_t_pc):
		if ((not attachee.has_met(triggerer)) or (group_percent_hp(triggerer) <= 30)):
			return RUN_DEFAULT
	return SKIP_DEFAULT


def smigmal_escape( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	game.timevent_add( smigmal_return, ( attachee, ), 7200000 )
	return RUN_DEFAULT


def smigmal_return( attachee ):
	attachee.object_flag_unset(OF_OFF)
	game.global_flags[144] = 1
	return RUN_DEFAULT


def smig_backup( attachee, triggerer ):
	assassin_1 = game.obj_create( 14782, location_from_axis( 623L, 455L ) )
	game.particles( 'sp-invisibility', assassin_1 )
	game.sound( 4032, 1 )
	assassin_1.turn_towards(game.party[0])
	assassin_2 = game.obj_create( 14783, location_from_axis( 613L, 463L ) )
	game.particles( 'sp-invisibility', assassin_2 )
	assassin_2.turn_towards(game.party[0])
	return RUN_DEFAULT


def smigmal_well( attachee, pc ):
	dice = dice_new("1d10+1000")
	attachee.heal( OBJ_HANDLE_NULL, dice )
	attachee.healsubdual( OBJ_HANDLE_NULL, dice )
	return RUN_DEFAULT


def smig_backup_2( attachee, triggerer ):
	assassin_1 = game.obj_create( 14782, location_from_axis( 621L, 471L ) )
	game.particles( 'sp-invisibility', assassin_1 )
	game.sound( 4032, 1 )
	assassin_1.turn_towards(game.party[0])
	assassin_2 = game.obj_create( 14783, location_from_axis( 634L, 472L ) )
	game.particles( 'sp-invisibility', assassin_2 )
	assassin_2.turn_towards(game.party[0])
	return RUN_DEFAULT
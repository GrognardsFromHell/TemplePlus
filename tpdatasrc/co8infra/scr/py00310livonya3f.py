from __main__ import game;
from toee import *
from utilities import *
from combat_standard_routines import *
from py00439script_daemon import set_v, get_v, set_f, get_f, inc_v


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[777] >= 1):
		attachee.destroy()
	if (game.global_vars[777] == 0):
		game.new_sid = 0
		game.global_vars[777] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if attachee.map == 5067 and attachee.name in [14174, 14175, 14177, 13002]: ## ToEE level 2 - big bugbear room scripting
		xx, yy = location_to_axis(attachee.location)
		if xx >= 416 and xx <= 434 and yy>= 350 and yy <= 398: # big bugbear room
			if get_v('bugbear_room_timevent_count') < 5:
				game.timevent_add( bugbear_room_increment_turn_counter, ( get_v('bugbear_room_turn_counter') + 1 ), 750 ) # reset flag in 750ms; in this version, time is frozen, so it will only take place next turn
				inc_v('bugbear_room_timevent_count')
			pcs_in_east_hallway = 0
			pcs_in_south_hallway = 0
			pcs_in_inner_south_hallway = 0
			
			yyp_east_max = 355
			xxp_inner_max = 416
			for obj in game.party[0].group_list():
				xxp, yyp = location_to_axis(obj.location)
				if yyp >= 355 and yyp <= 413 and xxp >= 405 and xxp <= 415 and obj.is_unconscious() == 0:
					pcs_in_east_hallway += 1
					if yyp > yyp_east_max:
						yyp_east_max = yyp
				if yyp >= 414 and yyp <= 422 and xxp >= 405 and xxp <= 455:
					pcs_in_south_hallway += 1
				if yyp >= 391 and yyp <= 413 and xxp >= 416 and xxp <= 455:
					pcs_in_inner_south_hallway += 1
					if xxp > xxp_inner_max:
						xxp_inner_max = xxp
					

			bugbears_near_door = []
			bugbears_near_south_entrance = []
			
			
			for bugbear_dude in game.obj_list_vicinity( location_from_axis(416, 359), OLC_NPC):
				if bugbear_dude.name in [14174, 14175, 14177] and willing_and_capable( bugbear_dude ):
					xxb, yyb = location_to_axis(bugbear_dude.location)
					if xxb >= 416 and xxb <= 434 and yyb >= 350 and yyb < 372:
						bugbears_near_door.append( bugbear_dude )
					## TODO - fear
					
			for bugbear_dude in game.obj_list_vicinity( location_from_axis(425, 383), OLC_NPC):
				if bugbear_dude.name in [14174, 14175, 14177] and willing_and_capable( bugbear_dude ):
					xxb, yyb = location_to_axis(bugbear_dude.location)
					if xxb >= 416 and xxb <= 434 and yyb >= 372 and yyb <= 399:
						bugbears_near_south_entrance.append( bugbear_dude )
						
			
			if pcs_in_inner_south_hallway == 0 and pcs_in_south_hallway == 0 and pcs_in_east_hallway > 0:
				# PCs in east hallway only - take 3 turns to get there
				if get_v('bugbear_room_turn_counter') >= 3:
					if yyp_east_max <= 395:
						yyb_base = yyp_east_max + 20
						xxb_base = 406
					else:
						xxb_base = 416
						yyb_base = 415
					bb_index = 0
					bb_x_offset_array = [0, 0, 1, 1, 2, 2, -1, -1 ]
					bb_y_offset_array = [0, 1, 0, 1, 0, 1,  0,  1 ]
					for bugbear_dude in bugbears_near_south_entrance:
						if bugbear_dude != attachee and bb_index <= 7:
							bugbear_dude.move( location_from_axis( xxb_base + bb_x_offset_array[bb_index], yyb_base + bb_y_offset_array[bb_index] ), 0.0, 0.0 )
							bugbear_dude.attack(game.leader)
							bb_index += 1

				

			elif pcs_in_inner_south_hallway > 0 and pcs_in_south_hallway == 0 and pcs_in_east_hallway == 0:
				#PCs in inner south hallway only - take 3 turns to reach
				if get_v('bugbear_room_turn_counter') >= 3:
					if xxp_inner_max <= 440:
						xxb_base = xxp_inner_max + 15
						yyb_base = 406
						bb_x_offset_array = [0,  0, 1,  1, 2,  2,  0,  0 ]
						bb_y_offset_array = [0, -1, 0, -1, 0, -1,  1,  2 ]
					else:
						xxb_base = 450
						yyb_base = 415
						bb_x_offset_array = [0, 0, 1, 1, 2, 2, -1, -1 ]
						bb_y_offset_array = [0, 1, 0, 1, 0, 1,  0,  1 ]
					bb_index = 0

					for bugbear_dude in bugbears_near_door:
						if bugbear_dude != attachee and bb_index <= 7:
							bugbear_dude.move( location_from_axis( xxb_base + bb_x_offset_array[bb_index], yyb_base + bb_y_offset_array[bb_index] ), 0.0, 0.0 )
							bugbear_dude.attack(game.leader)
							bb_index += 1


	## THIS IS USED FOR BREAK FREE
	#found_nearby = 0
	#for obj in game.party[0].group_list():
	#	if (obj.distance_to(attachee) <= 3 and obj.stat_level_get(stat_hp_current) >= -9):
	#		found_nearby = 1
	#if found_nearby == 0:
	#	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
	#		attachee.item_find(8903).destroy()
	#	if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#		create_item_in_inventory( 8903, attachee )
##		attachee.d20_send_signal(S_BreakFree)

	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	return RUN_DEFAULT



def bugbear_room_increment_turn_counter( turn_num):
	set_v('bugbear_room_turn_counter', turn_num )
	set_v('bugbear_room_timevent_count', 0)



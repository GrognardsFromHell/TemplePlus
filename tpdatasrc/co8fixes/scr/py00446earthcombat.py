from toee import *

from utilities import *

from Livonya import *

from py00439script_daemon import record_time_stamp, get_f, set_f, get_v, set_v, npc_set, npc_unset, npc_get, tsc, within_rect_by_corners, can_see_party, is_far_from_party, party_closest, encroach, tpsts

from Co8 import unequip


def san_dialog( attachee, triggerer ):
	if attachee.name == 14913: # grate
		triggerer.begin_dialog( attachee, 1000 )
	elif attachee.name == 14914: # barrier
		if game.global_flags[104] == 0 and not anyone( triggerer.group_list(), "has_follower", 8045) :
			triggerer.begin_dialog( attachee, 1500 )
		else:
			triggerer.begin_dialog( attachee, 2100)
	return SKIP_DEFAULT

	
def san_heartbeat(attachee, triggerer):
	pass

def san_enter_combat(attachee, triggerer):
	if attachee.name == 14913: # grate
		attachee.move( location_from_axis( 415, 556 ), 0, 11.0)
		attachee.rotation = 1.57*3/2

	tag_strategy(attachee)
	get_melee_strategy(attachee)


def san_exit_combat(attachee, triggerer):
	# attahcee.standpoint_set(STANDPOINT_DAY, attachee.obj_get_int(obj_f_npc_pad_i_3))
	# attahcee.standpoint_set(STANDPOINT_NIGHT, attachee.obj_get_int(obj_f_npc_pad_i_3))
	# set_v('Test666', get_v('Test666') | 2 )
	dummy = 1

	if game.global_flags[403] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 14, 1 )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if attachee.name == 14913: # harpy grate
		attachee.move( location_from_axis( 415, 556 ), 0, 11.0)
		attachee.rotation = 1.57*3/2
		return SKIP_DEFAULT
	elif attachee.name == 14914:
		return SKIP_DEFAULT
	elif attachee.name == 14811: # The Beacon
		attachee.obj_set_int(324, 104)
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 )
		return RUN_DEFAULT

	#################################
	# Copied from script 310 :	#
	#################################
	## THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)



	#if game.global_flags[403] == 1:
	#	attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 )


	if attachee.name == 14249 or attachee.name == 14381 or attachee.name == 14296:  # Ogre, Medium Earth Elem, Large Earth Elem
		get_melee_reach_strategy(attachee)
	elif attachee.name == 14243 and attachee.map == 5066: # harpies	at Temple Level 1
		ghouls_harpies_join_in( attachee )
		spawn_grate_object()
	else:
		get_melee_strategy(attachee)


def san_dying( attachee, triggerer = game.leader ):
	if attachee.name == 14913: # grating
		grate_obj = OBJ_HANDLE_NULL
		for door_candidate in game.obj_list_vicinity( attachee.location, OLC_PORTAL ):
			if (door_candidate.name == 120):
				grate_obj = door_candidate
		if grate_obj != OBJ_HANDLE_NULL:
			grate_obj.object_flag_set(OF_OFF)
			grate_obj.portal_flag_set(OPF_OPEN)
		game.particles( 'ef-MinoCloud', attachee )
		game.particles( 'Orb-Summon-Earth Elemental', attachee )
		game.particles( 'Mon-EarthElem-Unconceal', attachee )
		game.sound( 4042, 1 )
		attachee.object_flag_set(OF_OFF)
	elif attachee.name == 14914: #Earth Temple Barrier
		barrier_obj = OBJ_HANDLE_NULL
		for door_candidate in game.obj_list_vicinity( attachee.location, OLC_PORTAL):
			if (door_candidate.name == 121):
				door_candidate.portal_flag_set(OPF_OPEN)
				door_candidate.destroy()
		# game.particles( 'ef-MinoCloud', attachee )
		game.particles( 'Orb-Summon-Earth Elemental', attachee )
		game.particles( 'Mon-EarthElem-Unconceal', attachee )
		game.sound( 4042, 1 )
		attachee.object_flag_set(OF_CLICK_THROUGH)
		game.timevent_add( barrier_away, (attachee, 2), 100)
		game.timevent_add( barrier_away, (attachee, 3), 200)
		#attachee.object_flag_set(OF_OFF)





def san_end_combat( attachee, triggerer ):
	if attachee.name == 14811: # The Beacon
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 16, 1 )
		countt_encroachers = 1
		countt_all = 1
		for npc in game.obj_list_vicinity(attachee.location, OLC_NPC):
			attachee.float_mesfile_line( 'mes\\test.mes', countt_all, 1 )
			countt_all += 1
			if is_far_from_party(npc, 48) and npc.is_unconscious() == 0:
				attachee.float_mesfile_line( 'mes\\test.mes', countt_encroachers, 2 )
				countt_encroachers += 1
				joe = party_closest(npc)
				#encroach(npc, joe)
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 16, 1 )
		if get_v('Beacon_Active') > 0:
			set_v('Beacon_Active', get_v('Beacon_Active') - 1)
		attachee.destroy() #so combat won't get stuck
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 16, 1 )
		return RUN_DEFAULT












def san_enter_combat_backup_with_beacon_shit(attachee, triggerer):

	tag_strategy(attachee)


	if attachee.name == 14811: # The Beacon
		attachee.scripts[16] = 446 #end combat round script
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 13, 1 )
		return RUN_DEFAULT

	if ( can_see_party(attachee) == 0 and is_far_from_party(attachee, 10) == 1 ) or is_far_from_party(attachee, 40):
		if is_far_from_party(attachee, 70) == 1:
			joe = party_closest(attachee)
			encroach(attachee, joe)
		attachee.obj_set_int(324, 119) # Seek out low ac beacon
		game.particles( "sp-Hold Person", attachee )
		if get_v('Beacon_Active') == 0:
			top_path = 0
			bottom_path = 0
			for pc in game.party:
				if within_rect_by_corners(pc, 467, 360, 467, 388) and pc.is_unconscious() == 0:
					top_path = top_path + 1
				if within_rect_by_corners(pc,504,355,504,385) and pc.is_unconscious() == 0:
					bottom_path = bottom_path + 1
			if top_path > bottom_path:
				primary_beacon_x = 470
				primary_beacon_y = 388
				tertiary_beacon_x = 492
				tertiary_beacon_y = 387
			else:
				primary_beacon_x = 492
				primary_beacon_y = 387
				tertiary_beacon_x = 470
				tertiary_beacon_y = 388
			beacon_loc = location_from_axis(primary_beacon_x, primary_beacon_y)
			beacon3_loc = location_from_axis(tertiary_beacon_x, tertiary_beacon_y)

			beacon=game.obj_create( 14811, beacon_loc )
			beacon.move(location_from_axis(470,388),0,0)
			beacon.obj_set_int(obj_f_npc_ac_bonus, -50)
			beacon.stat_base_set(stat_dexterity,-70) #causes problems at end of round, or does it?
			#beacon.object_flag_set(OF_DONTDRAW) # this causes combat to lag at the beacon's turn
			beacon.object_flag_set(OF_CLICK_THROUGH)
			beacon.add_to_initiative()
			beacon.set_initiative(-20)
			game.update_combat_ui()
			beacon.scripts[16] = 446 #end combat round
			#beacon.scripts[14] = 446 # exit combat
			game.particles('sp-hold person', beacon)

			beacon2=game.obj_create( 14811, location_from_axis (483L, 395L) )
			beacon2.move(location_from_axis(483,395),0,0)
			beacon2.obj_set_int(obj_f_npc_ac_bonus, -40)
			#beacon2.object_flag_set(OF_DONTDRAW)
			beacon2.object_flag_set(OF_CLICK_THROUGH)
			beacon2.add_to_initiative()
			beacon2.set_initiative(-21)
			game.update_combat_ui()
			game.particles('sp-hold person', beacon2)

			beacon3=game.obj_create( 14811, beacon3_loc )
			beacon3.move(beacon3_loc,0,0)
			beacon3.obj_set_int(obj_f_npc_ac_bonus, -30)
			#beacon3.object_flag_set(OF_DONTDRAW)
			beacon3.object_flag_set(OF_CLICK_THROUGH)
			beacon3.add_to_initiative()
			beacon3.set_initiative(-23)
			game.update_combat_ui()
			game.particles('sp-hold person', beacon3)

			set_v('Beacon_Active',3)
	elif is_far_from_party(attachee, 75):
		joe = party_closest(attachee)
		encroach(attachee, joe)	
	else:
		get_melee_strategy(attachee)

	# Tried changing their standpoint midfight, didn't work.
	# attachee.standpoint_set(STANDPOINT_DAY, attachee.obj_get_int(obj_f_npc_pad_i_3) - 342 + 500)
	# attachee.standpoint_set(STANDPOINT_NIGHT, attachee.obj_get_int(obj_f_npc_pad_i_3) - 342 + 500)
	# if attachee.obj_get_int(obj_f_npc_pad_i_3) == 361 or attachee.obj_get_int(obj_f_npc_pad_i_3) == 362: #sentry standpoints
	#	hl(attachee)
	#	xx = 482
	#	yy = 417
	#	for npc in game.obj_list_vicinity(location_from_axis(xx,yy), OLC_NPC ):
	#		if npc.leader_get() == OBJ_HANDLE_NULL:
	#			npc.standpoint_set(STANDPOINT_DAY, npc.obj_get_int(obj_f_npc_pad_i_3) - 342 + 500)
	#			npc.standpoint_set(STANDPOINT_NIGHT, npc.obj_get_int(obj_f_npc_pad_i_3) - 342 + 500)	
	#			npc.npc_flag_set(ONF_KOS)
	#			npc.npc_flag_unset(ONF_KOS_OVERRIDE)
	return RUN_DEFAULT



def san_start_combat_with_beacon_shit( attachee, triggerer ): 
	if attachee.name == 14811: # The Beacon
		attachee.obj_set_int(324, 104)
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 ) 
		return RUN_DEFAULT
	#################################
	# Copied from script 310 :	#
	#################################
	## THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)

	if game.global_flags[403] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 ) 
#	if is_far_from_party(attachee, 48):
#		joe = party_closest(attachee)
#		encroach(attachee, joe)	
#	if can_see_party(attachee) == 0:
#		attachee.obj_set_int(324, 119) # Seek out hurt friend
#		if game.global_flags[403] == 1:
#			game.particles( "sp-bless water", attachee )
#	else:
#		if game.global_flags[403] == 1:
#			game.particles( "sp-summon monster I", attachee )
#		if attachee.name == 14249 or attachee.name == 14381 or attachee.name == 14296:
#			get_melee_reach_strategy(attachee)
#		else:
#			get_melee_strategy(attachee)

def ghouls_harpies_join_in( attachee ):			
	for obj in game.obj_list_vicinity(attachee.location, OLC_NPC):
		if obj.name in [14095, 14128, 14129] and within_rect_by_corners(obj, 415, 556, 419, 590) and obj.leader_get() == OBJ_HANDLE_NULL:
			# ghouls in the eastern room - should join the fray, per the module
			obj.attack(game.leader)
		if obj.name == 14243 and obj.leader_get() == OBJ_HANDLE_NULL:
			# other harpies
			obj.attack(game.leader)	
	if attachee.map == 5066 and (get_f('j_ghouls_corridor_temple_1') == 0): # temple level 1
		set_f('j_ghouls_corridor_temple_1')
		yyp_max = 556
		yyp_o = 561
		for pc in game.party[0].group_list():
			xxp, yyp = location_to_axis( pc.location )
			if xxp >= 433 and yyp >= 561:
				if yyp > yyp_max:
					yyp_max = yyp
		y_ghoul = yyp_max + 18
		y_ghoul = min( 603, y_ghoul )
		y_ghoul_add = [0, 0, 2, 2, -2]
		x_ghoul_add = [0, 2, 0, 2,  0]
		x_ghoul = 433
		ghoul_counter = 0
		
		for npc in game.obj_list_vicinity(location_from_axis(463, 603), OLC_NPC):
			if npc.name == 14129 and npc.leader_get() == OBJ_HANDLE_NULL and npc.is_unconscious() == 0:
				npc.move( location_from_axis( x_ghoul + x_ghoul_add[ghoul_counter] , y_ghoul + y_ghoul_add[ghoul_counter]), 0, 0)
				ghoul_counter += 1
				npc.attack(game.leader)
				if ghoul_counter >= len(y_ghoul_add):
					break

def spawn_grate_object():					
	if get_f('harpy_grate') == 0:
		should_drop_grate = 0
		for pc in game.party[0].group_list():
			xx,yy = location_to_axis( pc.location )
			if xx <= 415 and yy < 580 and yy > 500:
				should_drop_grate = 1
			if should_drop_grate:
				set_f('harpy_grate')
				grate_obj = OBJ_HANDLE_NULL
				for obj in game.obj_list_vicinity( location_from_axis (415L, 556L), OLC_PORTAL):
					if obj.name == 120:
						grate_obj = obj
				if grate_obj == OBJ_HANDLE_NULL:
					grate_obj = game.obj_create( 120, location_from_axis (415L, 556L) )
					grate_obj.move( location_from_axis (415L, 556L), 0, 11.0 )
					grate_obj.rotation = 1.57*3/2
					grate_obj.portal_flag_set(OPF_JAMMED)
					grate_obj.object_flag_set(OF_SHOOT_THROUGH)
					grate_obj.object_flag_set(OF_SEE_THROUGH)
					grate_obj.object_flag_set(OF_CLICK_THROUGH)
					grate_obj.object_flag_set(OF_TRANSLUCENT)
					grate_obj.object_flag_set(OF_NO_BLOCK)
					grate_obj.object_flag_set(OF_NOHEIGHT)
				
				#game.timevent_add( grate_npc_timed_event, (  ), 250, 1 )
				game.particles("Orb-Summon-Earth Elemental", grate_obj)
				game.shake(50, 500)
				game.sound(4180, 1)

def grate_strength():
	strength_array = []
	for obj in game.party[0].group_list():
		strength_array.append( obj.stat_level_get(stat_strength) )
	strength_array.sort()
	strength_array.reverse()
	if strength_array[0] >= 24:
		return 2
	if len(strength_array) >= 3:
		if strength_array[0] + strength_array[1] + strength_array[2] >= 60:
			return 1
	return 0

def grate_away( npc ):
	npc.object_script_execute( npc, 12 )
	
def grate_npc_timed_event():
	grate_npc = game.obj_create( 14913, location_from_axis (415L, 556L) )
	grate_npc.move( location_from_axis (415L, 556L), 0, 11.0 )
	grate_npc.rotation = 1.57*3/2
	grate_npc.object_flag_set(OF_SHOOT_THROUGH)
	grate_npc.object_flag_set(OF_SEE_THROUGH)
	
	
def earth_temple_listen_check():
	highest_listen_mod = -10
	for pc in game.party:
		if pc.type == obj_t_pc:
			if pc.skill_level_get(skill_listen) < highest_listen_mod:
				highest_listen_mod = pc.skill_level_get(skill_listen)
	game.global_vars[35] = game.random_range(1,20) + highest_listen_mod
	
	
def earth_temple_haul_inside(pc, npc, line):

	game.timevent_add( move_party_inside, (pc, npc, 1555, 480,393), 181150 )
	game.fade(180,0,0,1)



def move_party_inside( pc, npc, line, x, y ):
	game.fade_and_teleport(30,0,0,5066,x,y)
	#game.global_vars[1] = 400
	game.timevent_add( talk_to_gatekeeper, (pc, npc, line, x, y), 100 )
	return

def talk_to_gatekeeper(pc, npc, line, x, y):
	operator = game.obj_create( 14915, location_from_axis (x+1, y+1) )
	operator.scripts[9] = 446
	operator.scripts[10] = 446
	operator.scripts[19] = 0
	operator.object_flag_set(OF_DONTDRAW)
	#game.global_vars[1] = 401
	if npc.name == 14915:
		#game.global_vars[1] = 402
		npc.destroy()
		#game.global_vars[1] = 403
	pc.begin_dialog(operator, line)
	#game.global_vars[1] += 1000
	#game.particles( "sp-summon monster I", operator )
	return


def switch_to_npc(pc, originator_npc = OBJ_HANDLE_NULL, npc_name = 0, dest_line = 0, failsafe_line = 0):
	if npc_name == 'romag':
		npc_name = 8045
	elif npc_name == 'hartsch':
		npc_name = 14154
	elif npc_name == 'earth troop commander':
		npc_name = 14156
	else:
		npc_name = 0 # failure
	for obj in game.obj_list_vicinity(pc.location, OLC_NPC):
		if obj.name == npc_name: 
			if dest_line == 0:
				if originator_npc.name == 14915: # sentinel
					originator_npc.destroy()
				obj.object_script_execute( pc, 9)
				return
			else: 
				if originator_npc.name == 14915: # sentinel
					originator_npc.destroy()
				pc.begin_dialog(obj, dest_line)
				return
	if current_line == 0:
		originator_npc.object_script_execute( pc, 9 )
	else:
		pc.begian_dialog(originator_npc, current_line)
	return
			
			
def earth_attack_party(pc, npc):
	if npc.name == 14915:
		game.timevent_add( npc.destroy, (), 100, 1 )
	for obj in game.obj_list_vicinity(pc.location, OLC_NPC):
		if not (obj in game.leader.group_list() ) and obj.name != 14914 and obj.name != 14915:
			obj.attack(pc)
	return
			
def barrier_away( npc = OBJ_HANDLE_NULL, barrier_smash_stage = 1 ):
	if barrier_smash_stage == 1:
		xx, yy = location_to_axis(npc.location)
		if xx <= 480:
			xxo = 500
		else:
			xxo = 460
		game.global_vars[2] += 100
		for other_barrier in game.obj_list_vicinity( location_from_axis(xxo, 378), OLC_NPC):
			if other_barrier.name == 14914:
				game.timevent_add(other_barrier.object_script_execute, (other_barrier, 12), game.random_range(100, 200), 1)
				#other_barrier.object_script_execute(other_barrier, 12)
				#other_barrier.object_flag_set(OF_OFF)
				game.timevent_add(other_barrier.object_flag_set, (OF_OFF), game.random_range(200, 300), 1)
		npc.object_script_execute( npc, 12 )


	elif barrier_smash_stage == 2:
		#game.timevent_add( barrier_away, ( OBJ_HANDLE_NULL, 3), 70, 1 )
		pcs_in_northern_access = 0
		pcs_in_southern_access = 0
		pcs_in_northern_hallway = 0
		pcs_in_southern_hallway = 0
		yy_south_max = 300
		yy_north_max = 300

		for pc in game.party:
			xx, yy = location_to_axis(pc.location)
			if (xx >= 461 and xx <= 470 and yy <= 379 and yy>= 370) and willing_and_capable(pc):
				pcs_in_northern_access += 1
			if (xx>= 494 and xx<= 505 and yy>= 370 and yy<= 379):
				pcs_in_southern_access += 1
			if ( xx >= 451 and xx < 461 and yy >= 370 and yy <= 440 ):
				pcs_in_northern_hallway += 1
				if yy > yy_north_max - 18:
					yy_north_max = min( yy + 18, 442)
			if ( xx > 505 and xx <= 515 and yy >= 370 and yy <= 442):
				pcs_in_southern_hallway += 1
				if yy > yy_south_max - 18:
					yy_south_max = min( yy + 18, 444)

		if pcs_in_northern_access > 0 and pcs_in_southern_access == 0 and pcs_in_southern_hallway == 0 and pcs_in_northern_hallway == 0:
			## PCs in northern access
			y_troop = 378 + 17
			x_troop = 459
			x_troop_add = [0, -2,  0, -2, -1, -3, -4, -5, -5, -1]
			y_troop_add = [0,  0, -2, -2, -1, -1, -2, -3, -1,  1]
		
		elif pcs_in_northern_access == 0 and pcs_in_southern_access > 0 and pcs_in_southern_hallway == 0 and pcs_in_northern_hallway == 0:
			## PCs in southern access
			y_troop = 378 + 17
			x_troop = 506
			x_troop_add = [0,  2,  0,  2,  1,  3,  4,  5,  5,  1]
			y_troop_add = [0,  0, -2, -2, -1, -1, -2, -3, -1,  1]
		
		elif  pcs_in_northern_hallway > 0 and pcs_in_northern_hallway >= pcs_in_southern_hallway:
			## PCs in Northern hallway
			y_troop = yy_north_max
			x_troop = 459
			x_troop_add = [0, -2,  0, -2, -1, -3, -4, -5, -5, -1]
			y_troop_add = [0,  0, -2, -2, -1, -1, -2, -3, -1,  1]
		
		elif  pcs_in_southern_hallway > 0:
			## PCs in Southern hallway
			y_troop = yy_south_max
			x_troop = 506
			x_troop_add = [0,  2,  0,  2,  1,  3,  4,  5,  5,  1]
			y_troop_add = [0,  0, -2, -2, -1, -1, -2, -3, -1,  1]

		N_max = len(y_troop_add) 
		troop_counter = 0
		if x_troop < 470:
			# use the ones near the south, or at the back
			for obj in game.obj_list_vicinity(location_from_axis(495, 389), OLC_NPC):
				xx, yy = location_to_axis(obj.location)
				if obj.leader_get() == OBJ_HANDLE_NULL and obj.name in [8045, 14066,  14078, 14154, 14156, 14162, 14163, 14164, 14165,  14248, 14249, 14296, 14337, 14338, 14339, 14381]:
					if obj.name in [14296, 14381]:
						obj.obj_set_int(obj_f_speed_run, 1075644444)
						obj.obj_set_int(obj_f_speed_walk, 1075064444)
						game.timevent_add(obj.unconceal, (), 450, 1)
					if xx >= 483 and xx <= 506 and troop_counter < N_max:
						obj.move( location_from_axis( x_troop + x_troop_add[troop_counter] , y_troop + y_troop_add[troop_counter]), 0, 0)
						troop_counter += 1
					elif xx >= 483 and xx <= 506:
						obj.move( location_from_axis( xx - 15 , yy), 0, 0)
					elif xx >=475 and xx < 483 and yy <= 392:
						obj.move( location_from_axis( xx - 4 , yy), 0, 0)
					elif xx >=475 and xx < 483 and yy > 392:
						obj.move( location_from_axis( xx - 4 , yy - 10), 0, 0)
					elif xx < 475 and yy > 392:	
						obj.move( location_from_axis( xx , yy - 10), 0, 0)
					#obj.attack(game.leader)  # fucks up the script
					if game.random_range(1, 100) <= 40:
						mang = party_closest(obj)
						game.timevent_add(obj.attack, ( mang ), game.random_range( 1, 1500 ), 1)
		else:
			# use the ones near the north, or at the back
			for obj in game.obj_list_vicinity(location_from_axis(465, 389), OLC_NPC):
				xx, yy = location_to_axis(obj.location)
				if obj.leader_get() == OBJ_HANDLE_NULL and obj.name in [8045, 14066,  14078, 14154, 14156, 14162, 14163, 14164, 14165,  14248, 14249, 14296, 14337, 14338, 14339, 14381]:
					if obj.name in [14296, 14381]: # Earth Elementals
						obj.obj_set_int(obj_f_speed_run, 1075644444)
						obj.obj_set_int(obj_f_speed_walk, 1075064444)
						game.timevent_add(obj.unconceal, (), 450, 1)
					if xx >= 459 and xx <= 480 and troop_counter < N_max:
						obj.move( location_from_axis( x_troop + x_troop_add[troop_counter] , y_troop + y_troop_add[troop_counter]), 0, 0)
						troop_counter += 1
					elif xx >= 459 and xx <= 477:
						obj.move( location_from_axis( xx + 15 , yy), 0, 0)
					elif xx > 477 and xx <= 485 and yy <= 392:
						obj.move( location_from_axis( xx + 4 , yy), 0, 0)
					elif xx > 477 and xx <= 485 and yy > 392:
						obj.move( location_from_axis( xx + 4 , yy - 10), 0, 0)
					elif xx > 477 and yy > 392:	
						obj.move( location_from_axis( xx , yy - 10), 0, 0)
					#obj.attack(game.leader)  # fucks up the script
					if game.random_range(1, 100) <= 40:
						mang = party_closest(obj)
						game.timevent_add(obj.attack, ( mang ), game.random_range( 1, 1500 ), 1)
			
	elif barrier_smash_stage == 3:
		#game.timevent_add(barrier_away, (OBJ_HANDLE_NULL, 3), 120, 1)
		if npc != OBJ_HANDLE_NULL and npc.name == 14914:
			npc.object_flag_set(OF_OFF)
		for obj in game.obj_list_vicinity( location_from_axis(470, 389), OLC_NPC):
			xx, yy = location_to_axis(obj.location)
			if game.random_range(1,100) <= 40 and xx >= 453 and xx <= 513 and obj.name in [8045, 14078, 14066, 14162, 14163, 14164, 14165, 14337, 14339, 14156, 14381, 14153, 14154] and obj.leader_get() == OBJ_HANDLE_NULL:
				mang = party_closest(obj)
				game.timevent_add(obj.attack, ( mang ), game.random_range( 200, 2200 ), 1)
	elif barrier_smash_stage == 4:
		#game.timevent_add(barrier_away, (OBJ_HANDLE_NULL, 3), 120, 1)
		if npc != OBJ_HANDLE_NULL and npc.name == 14914:
			npc.object_flag_set(OF_OFF)

	

		

def switch_to_gatekeeper(pc, line):
	xx, yy = location_to_axis(pc.location)
	if xx >= 466 and xx <= 501 and yy >= 378 and yy <= 420:
		gate_keeper_npc = game.obj_create(14915, pc.location)
		gate_keeper_npc.scripts[15] = 446
		gate_keeper_npc.scripts[13] = 446
		gate_keeper_npc.scripts[19] = 0
		gate_keeper_npc.scripts[9] = 446
		gate_keeper_npc.object_flag_set(OF_DONTDRAW)
		gate_keeper_npc.object_flag_set(OF_CLICK_THROUGH)
		pc.begin_dialog(gate_keeper_npc, line)
			
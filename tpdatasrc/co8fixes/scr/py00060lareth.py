from __main__ import game;
from toee import *
from py00439script_daemon import *
from combat_standard_routines import *
	
	
# pad3: internal flags
# 2**1 - have cast Dispell

	
def san_dialog( attachee, triggerer ):
	game.global_flags[834] = 1 
	if (anyone( triggerer.group_list(), "has_item", 11003 ) and attachee.item_find(11003) == OBJ_HANDLE_NULL):
		if ( game.global_flags[198] == 1 ):
			triggerer.begin_dialog( attachee, 1000 )
		elif (attachee.leader_get() != OBJ_HANDLE_NULL):
			if (game.global_flags[53] == 1):
				triggerer.begin_dialog( attachee, 1100 )
			else:
				triggerer.begin_dialog( attachee, 1200 )
		elif ( game.global_flags[52] == 1 ):
			triggerer.begin_dialog( attachee, 1300 )
		elif ( game.global_flags[48] == 1 ):
			triggerer.begin_dialog( attachee, 1400 )
		else:
			triggerer.begin_dialog( attachee, 1500 )
	elif ( game.global_flags[198] == 1 ):
		triggerer.begin_dialog( attachee, 260 )
	elif (attachee.leader_get() != OBJ_HANDLE_NULL):
		if (game.global_flags[53] == 1):
			triggerer.begin_dialog( attachee, 320 )
		else:
			triggerer.begin_dialog( attachee, 220 )
	elif ( game.global_flags[52] == 1 ):
		triggerer.begin_dialog( attachee, 20 )
	elif ( game.global_flags[48] == 1 ):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 10 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not attachee.has_spell_effects() and (game.party_alignment == LAWFUL_NEUTRAL or game.party_alignment == LAWFUL_EVIL or game.party_alignment == LAWFUL_GOOD or game.party_alignment == TRUE_NEUTRAL)):
		attachee.cast_spell(spell_detect_law, attachee)
		attachee.spells_pending_to_memorized()
		game.global_vars[726] = 2
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not attachee.has_spell_effects() and (game.party_alignment == CHAOTIC_NEUTRAL or game.party_alignment == CHAOTIC_GOOD or game.party_alignment == CHAOTIC_EVIL)):
		attachee.cast_spell(spell_detect_chaos, attachee)
		attachee.spells_pending_to_memorized()
		game.global_vars[726] = 2
	if (game.party_alignment == NEUTRAL_GOOD and attachee.leader_get() == OBJ_HANDLE_NULL and not attachee.has_spell_effects()):
		attachee.cast_spell(spell_detect_good, attachee)
		attachee.spells_pending_to_memorized()
		game.global_vars[726] = 2
	if (game.party_alignment == NEUTRAL_EVIL and attachee.leader_get() == OBJ_HANDLE_NULL and not attachee.has_spell_effects()):
		attachee.cast_spell(spell_detect_evil, attachee)
		attachee.spells_pending_to_memorized()
		game.global_vars[726] = 2
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
#	if game.global_flags[277] == 0:
##Kalshane's idea for Raimol ratting the party out to the traders and triggering the assassination - put on hold
#		for obj in game.party:
#			if obj.name == 8050:
#				a = game.encounter_queue
#				b = 1
#				for enc_id in a:
#					if enc_id == 3000:
#						b = 0
#				if b == 1:
#					game.encounter_queue.append(3000)
#					game.global_flags[420] = 1
	record_time_stamp(425)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
		game.global_flags[37] = 1
		if ( game.story_state <= 1 ):
			game.story_state = 2
		return RUN_DEFAULT
	attachee.float_line(12014,triggerer)
	game.global_flags[37] = 1
	if ( game.story_state <= 1 ):
		game.story_state = 2
	for pc in game.party:
		if ( pc.reputation_has( 18 ) == 1):
			pc.reputation_remove( 18 )
	attachee.float_line(12014,triggerer)
	game.party[0].reputation_add( 15 )
	if ( game.global_flags[340] == 1 ):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				game.global_flags[834] = 1 
				obj.begin_dialog( attachee, 370 )
				attachee.float_line(12014,triggerer)
	elif ( game.global_flags[62] == 0 ):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				game.global_flags[834] = 1 
				obj.begin_dialog( attachee, 390 )
				attachee.float_line(12014,triggerer)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer, generated_from_timed_event_call = 0 ):
	attachee.float_line(12058,triggerer) # "You have earned my wrath!"
	
	if (attachee.leader_get() == OBJ_HANDLE_NULL and (not attachee.has_wielded(1) or not attachee.has_wielded(4071))):
		attachee.item_wield_best_all()
		game.new_sid = 0
		
	if attachee.map == 5005:
		ggv400 = game.global_vars[400]
		if (ggv400 & (2**0)) == 0:
			ggv400 |= 2**5
			game.global_vars[400] = ggv400
		for obj in game.obj_list_vicinity(location_from_axis(487, 537), OLC_NPC):
			if obj.name in range(14074,  14078) and obj.leader_get() == OBJ_HANDLE_NULL:
				obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
				obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
				if triggerer.type == obj_t_pc or triggerer.leader_get() != OBJ_HANDLE_NULL:
					obj.attack(triggerer)
				else:
					obj.attack(game.leader)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer, generated_from_timed_event_call = 0, talk_stage = -999 ):
	if attachee.is_unconscious() == 1:
		return RUN_DEFAULT
	
	#if generated_from_timed_event_call == 0 and attachee.distance_to( party_closest(attachee) ) > 45:
	#	attachee.move( party_closest(attachee).location, 0 , 0)
	#	for pp in range(0, 41):
	#		attachee.scripts[pp] = 998	
	#	game.leader.ai_follower_add(attachee)
	#	#game.timevent_add( lareth_abandon, (attachee, triggerer), 20, 1)
	#	

		
	
	curr = attachee.stat_level_get( stat_hp_current ) - attachee.stat_level_get(stat_subdual_damage)
	maxx = attachee.stat_level_get( stat_hp_max )
	xx = attachee.location & ( 0xFFFF )
	hp_percent_lareth = 100* curr / maxx
	ggv400 = game.global_vars[400]
	ggv401 = game.global_vars[401]
	pad3 = attachee.obj_get_int(obj_f_npc_pad_i_3)

	if (attachee.map == 5005)   and   ( party_too_far_from_lareth( attachee ) )   and   ( generated_from_timed_event_call == 0 ) : 
		if ( ( pad3 & (2**2) ) == 0 ):
			# Delay for one round, letting him cast Shield of Faith - he'll need it :)
			pad3 |= (2**2)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
			return
		# Party is too far from Lareth, gotta nudge him in the right direction
		# spawn a beacon and change Lareth's strat to approach it
		if xx > 478:
			beacon_loc = 498L + ( (550L) << 32 )
		else:
			beacon_loc = 487L + ( (540L) << 32 )
		obj_beacon = game.obj_create(14074, beacon_loc)
		obj_beacon.object_flag_set(OF_DONTDRAW)
		obj_beacon.object_flag_set(OF_CLICK_THROUGH)
		obj_beacon.move(beacon_loc ,0,0)
		obj_beacon.stat_base_set( stat_dexterity, -30 )
		obj_beacon.obj_set_int(obj_f_npc_pad_i_3, 2**8 )
		obj_beacon.attack(game.leader)
		obj_beacon.add_to_initiative()

		attachee.obj_set_int(obj_f_pad_i_0, attachee.obj_get_int(obj_f_critter_strategy) ) # Record original strategy		
		attachee.obj_set_int(obj_f_critter_strategy, 80) # set Lareth's strat to "seek beacon"
		
		grease_detected = 0
		for spell_obj in game.obj_list_cone(closest_pc_1, OLC_GENERIC, 40, 0, 360):
			# Check for active GREASE spell object
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 200 + (1<<15):
				grease_detected = 1
				grease_obj = spell_obj
		if grease_detected:
			# In case Lareth slips and doesn't execute his san_end_combat (wherein the beacon object is destroyed) - spawn a couple of timed events to guarantee the beacon doesn't survive
			game.timevent_add( kill_beacon_obj, ( obj_beacon, attachee ), 3700, 1 )
			game.timevent_add( kill_beacon_obj, ( obj_beacon, attachee ), 3900, 1 )
		return RUN_DEFAULT
	
	# strategy 81 - Approach Party strategy
	if attachee.obj_get_int(obj_f_critter_strategy) == 81 and generated_from_timed_event_call == 0:
		if can_see_party(attachee):
			attachee.obj_set_int(obj_f_critter_strategy, 82)

	if attachee.obj_get_int(obj_f_critter_strategy) != 81 and generated_from_timed_event_call == 0:
		# Should Lareth cast Obscuring Mist?
		# First, find closest party member - the most likely target for an archer
		closest_pc_1 = game.leader
		for pc in game.party[0].group_list():
			if pc.distance_to(attachee) < closest_pc_1.distance_to(attachee):
				closest_pc_1 = pc
		
		# Then, check for spell objects with the Obscuring Mist ID, which are also identified as active
		player_in_obscuring_mist = 0
		for spell_obj in game.obj_list_cone(closest_pc_1, OLC_GENERIC, 30, 0, 360):
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 333 + (1<<15) and spell_obj.distance_to(closest_pc_1) <= 17.5:
				player_in_obscuring_mist = 1
				
		player_cast_web = 0
		player_cast_entangle = 0
		for spell_obj in game.obj_list_vicinity(attachee.location, OLC_GENERIC):
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 531 + (1<<15):
				player_cast_web = 1
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 153 + (1<<15):
				player_cast_entangle = 1					
		
		# Assess level of ranged weapon threat
		ranged_threat = 0
		for pc in game.party[0].group_list():
			pc_weap = pc.item_worn_at(3).obj_get_int(obj_f_weapon_type) 
			if pc_weap in [14, 17, 46, 48, 68] and pc.is_unconscious() == 0:
				# 14 - light crossbow
				# 17 - heavy crossbow
				# 46 - shortbow
				# 48 - longbow
				# 68 - repeating crossbow
				if ranged_threat == 0:
					ranged_threat = 1
				if  pc.has_feat(feat_point_blank_shot) or ( pc.stat_level_get(stat_level_fighter) + pc.stat_level_get(stat_level_ranger) ) >= 1 :
					if ranged_threat < 2:
						ranged_threat = 2
				if  pc.has_feat(feat_precise_shot) and ( pc.stat_level_get(stat_level_fighter) + pc.stat_level_get(stat_level_ranger) ) >= 1 :
					if ranged_threat < 3:
						ranged_threat = 3
						
		if (attachee.map == 5005 and xx > 478 ) and ( (ggv401 >> 25) & 3 == 0 ) and  (   (ranged_threat == 3)  or  ( ranged_threat > 1 and player_in_obscuring_mist == 1 )  or ( ranged_threat > 0 and ( player_cast_entangle or player_cast_web) )  ):
			# Cast Obscuring Mist, if:
			#  1. Haven't cast it yet  - (ggv401 >> 25) & 3
			#  2. Ranged threat exists (emphasized when player casts web or is in obscuring mist)
			# Give him a potion of Obscuring Mist, to simulate him having that scroll (just like I do...)
			if attachee.item_find_by_proto(8899) == OBJ_HANDLE_NULL:
				attachee.item_get( game.obj_create( 8899, attachee.location ) )
			ggv401 += 1<<25
			game.global_vars[401] = ggv401
			lareth_is_threatened = 0
			if closest_pc_1.distance_to(attachee) <= 3:
				lareth_is_threatened = 1
			if lareth_is_threatened == 1:
				attachee.obj_set_int(obj_f_critter_strategy, 85) # Obscuring Mist + 5ft step
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 86) # Just Obscuring Mist
		elif ( pad3 & (2**1) == 0 ) and ( player_cast_entangle or player_cast_web ):
			attachee.obj_set_int(obj_f_critter_strategy, 87) # Dispel strat
			pad3 |= (2**1)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
		elif attachee.map == 5005 and player_entrenched_in_corridor( attachee ):
			attachee.obj_set_int(obj_f_critter_strategy, 89)
		else:
			attachee.obj_set_int(obj_f_critter_strategy, 82)


	if ( hp_percent_lareth < 50) and ( generated_from_timed_event_call == 0 or generated_from_timed_event_call == 1 and talk_stage == 667):
		
		if ( ggv400 & (2**6) ) == 0:
			found_pc = OBJ_HANDLE_NULL
			obj_list = game.obj_list_vicinity( attachee.location, OLC_NPC)
			
			
			# Extending the range a little...
			
			for obj in game.obj_list_vicinity( attachee.location - 35, OLC_NPC):
				if not (obj in obj_list):
					obj_list += (obj,)

			for obj in game.obj_list_vicinity( attachee.location + 35, OLC_NPC):
				if not (obj in obj_list):
					obj_list += (obj,)
			

			
			for obj in obj_list:
				for pc in game.leader.group_list():
					if pc.type == obj_t_pc and pc.is_unconscious() == 0:
						found_pc = pc
					#if obj.name in ([attachee.name] + range(14074, 14078)):
					obj.ai_shitlist_remove( pc )
					obj.reaction_set( pc, 50 )
					obj.remove_from_initiative(obj)
					if pc.type == obj_t_npc:
						pc.ai_shitlist_remove( obj )
				for obj2 in game.obj_list_vicinity( attachee.location, OLC_ALL):
					if obj2.type == obj_t_pc or obj2.type == obj_t_npc:
						obj2.reaction_set( obj, 50 )
						try:
							obj2.ai_shitlist_remove(obj)
						finally:
							dummy = 1
						obj2.remove_from_initiative(obj2)


			if generated_from_timed_event_call == 0:
				game.timevent_add(san_start_combat, (attachee, triggerer, 1, 667), 100, 1)
			elif found_pc != OBJ_HANDLE_NULL:
				ggv400 |= 2**6
				game.global_vars[400] = ggv400
				game.global_flags[834] = 1 
				found_pc.begin_dialog( attachee, 160 )
				return SKIP_DEFAULT
		


	elif generated_from_timed_event_call == 0 and game.global_flags[834] == 0:
		if ( (ggv401>> 15) & 7) == 0:
			ggv401 += 1 << 15
			game.global_vars[401] = ggv401
		elif ( (ggv401>> 15) & 7) == 1:
			closest_distance_1 = game.leader.distance_to(attachee)
			for pc in game.party:
				closest_distance_1 = min( closest_distance_1, pc.distance_to(attachee) )
			if closest_distance_1 < 45:
				for ppq in range(3, 26):
					game.timevent_add(san_start_combat, (attachee, triggerer, 1, ppq), ppq*2500 + game.random_range(0,20), 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401

	elif generated_from_timed_event_call == 1 and game.global_flags[834] == 0:
		if ( hp_percent_lareth > 75 ) and (ggv400 & (2**4)) == 0 and attachee.d20_query(Q_Prone) == 0:
			if talk_stage >= 3 and ( (ggv401 >> 15) & 31 ) == 2:
				attachee.float_line(6000, triggerer)
				game.sound(4201, 1)
				game.sound(4201, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 3 and ( (ggv401 >> 15) & 31 ) == 3:
				game.sound(4202, 1)
				game.sound(4202, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 3 and ( (ggv401 >> 15) & 31 ) == 4:
				game.sound(4203, 1)
				game.sound(4203, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401

			elif talk_stage >= 8 and ( (ggv401 >> 15) & 31 ) == 5:
				attachee.float_line(6001, triggerer)
				game.sound(4204, 1)
				game.sound(4204, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 8 and ( (ggv401 >> 15) & 31 ) == 6:
				game.sound(4205, 1)
				game.sound(4205, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401

			elif talk_stage >= 13 and ( (ggv401 >> 15) & 31 ) == 7:
				attachee.float_line(6002, triggerer)
				game.sound(4206, 1)
				game.sound(4206, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 13 and ( (ggv401 >> 15) & 31 ) == 8:
				game.sound(4207, 1)
				game.sound(4207, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401


			elif talk_stage >= 18 and ( (ggv401 >> 15) & 31 ) == 9:
				attachee.float_line(6003, triggerer)
				game.sound(4208, 1)
				game.sound(4208, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 18 and ( (ggv401 >> 15) & 31 ) == 10:
				game.sound(4209, 1)
				game.sound(4209, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401

			elif talk_stage >= 22 and ( (ggv401 >> 15) & 31 ) == 11:
				attachee.float_line(6004, triggerer)
				game.sound(4210, 1)
				game.sound(4210, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 22 and ( (ggv401 >> 15) & 31 ) == 12:
				attachee.float_line(6004, triggerer)
				game.sound(4211, 1)
				game.sound(4211, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401
			elif talk_stage >= 22 and ( (ggv401 >> 15) & 31 ) == 13:
				attachee.float_line(6004, triggerer)
				game.sound(4212, 1)
				game.sound(4212, 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401


		elif ( hp_percent_lareth <= 75 ) and (ggv400 & (2**4)) == 0:
			if ( (ggv401 >> 15) & 31 ) > 2:
				attachee.float_line( 6005, triggerer)
				game.sound(4200, 1)
				game.sound(4200, 1)
			game.timevent_add(san_start_combat, (attachee, triggerer, 1, 667), 5500, 1)
			ggv400 |= 2**4
			game.global_vars[400] = ggv400

	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	
	return RUN_DEFAULT


def san_end_combat(attachee, triggerer, generated_from_timed_event_call = 0):
	if attachee.is_unconscious() == 1:
		return RUN_DEFAULT
	curr = attachee.stat_level_get( stat_hp_current ) - attachee.stat_level_get(stat_subdual_damage)
	maxx = attachee.stat_level_get( stat_hp_max )
	xx = attachee.location & ( 0xFFFF )
	hp_percent_lareth = 100* curr / maxx
	ggv400 = game.global_vars[400]
	ggv401 = game.global_vars[401]
	if generated_from_timed_event_call == 0 and game.global_flags[834] == 0:
		if ( (ggv401>> 15) & 7) == 1:
			closest_distance_1 = game.leader.distance_to(attachee)
			for pc in game.party:
				closest_distance_1 = min( closest_distance_1, pc.distance_to(attachee) )
			
			if closest_distance_1 <= 25 or xx > 480:
				for ppq in range(3, 26):
					game.timevent_add(san_start_combat, (attachee, triggerer, 1, ppq), ppq*2500 + game.random_range(0,20), 1)
				ggv401 += 1 << 15
				game.global_vars[401] = ggv401

	if generated_from_timed_event_call == 0:
		# Wake up sleeping guy script
		bbb = attachee.obj_get_int(obj_f_critter_strategy)
		if bbb == 80: # if using the 'seek beacon' strategy
			bbb = attachee.obj_get_int(obj_f_pad_i_0) # retrieve previous strat
			attachee.obj_set_int(obj_f_critter_strategy, bbb)
			for obj in game.obj_list_cone(attachee, OLC_NPC, 20, 0, 360):
				if obj.name in range(14074, 14078) and obj != attachee:
					obj_pad3 = obj.obj_get_int(obj_f_npc_pad_i_3)
					if obj_pad3 & (2**8) != 0: # is a beacon object
						obj.destroy()
	#	if can_see_party(attachee):
	#		flash_signal(10)
	#		attachee.obj_set_int(obj_f_critter_strategy, 82)
	#	game.timevent_add(san_end_combat, ( attachee, triggerer, 1), 300, 1)
	#else:
	#	if can_see_party(attachee):
	#		flash_signal(10)
	#		attachee.obj_set_int(obj_f_critter_strategy, 82)
	dummy = 1
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[37] = 0
	for pc in game.party:
		if ( pc.reputation_has( 15 ) == 1):
			pc.reputation_remove( 15 )
	for pc in game.party:
		if ( pc.reputation_has( 18 ) == 0):
			pc.reputation_add( 18 )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	game.global_vars[726] = game.global_vars[726] + 1
	if (game.combat_is_active()):
		return RUN_DEFAULT
	if (attachee.leader_get() == OBJ_HANDLE_NULL and (not attachee.has_wielded(1) or not attachee.has_wielded(4071))):
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
	if (game.global_vars[726] == 1 and attachee.leader_get() == OBJ_HANDLE_NULL and (game.party_alignment == LAWFUL_NEUTRAL or game.party_alignment == LAWFUL_EVIL or game.party_alignment == LAWFUL_GOOD or game.party_alignment == TRUE_NEUTRAL)):
		attachee.cast_spell(spell_detect_law, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[726] == 1 and attachee.leader_get() == OBJ_HANDLE_NULL and (game.party_alignment == CHAOTIC_NEUTRAL or game.party_alignment == CHAOTIC_GOOD or game.party_alignment == CHAOTIC_EVIL)):
		attachee.cast_spell(spell_detect_chaos, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[726] == 1 and game.party_alignment == NEUTRAL_GOOD and attachee.leader_get() == OBJ_HANDLE_NULL):
		attachee.cast_spell(spell_detect_good, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[726] == 1 and game.party_alignment == NEUTRAL_EVIL and attachee.leader_get() == OBJ_HANDLE_NULL):
		attachee.cast_spell(spell_detect_evil, attachee)
		attachee.spells_pending_to_memorized()
	for obj in game.party[0].group_list():
		if (attachee.distance_to(obj) <= 10 and game.global_vars[726] == 5):
			if (attachee.leader_get() == OBJ_HANDLE_NULL and (game.party_alignment == LAWFUL_NEUTRAL or game.party_alignment == LAWFUL_EVIL or game.party_alignment == LAWFUL_GOOD or game.party_alignment == TRUE_NEUTRAL)):
				attachee.cast_spell(spell_magic_circle_against_law, attachee)
				attachee.spells_pending_to_memorized()
#				return RUN_DEFAULT
	for obj in game.party[0].group_list():
		if (attachee.distance_to(obj) <= 10 and game.global_vars[726] == 5):
			if (attachee.leader_get() == OBJ_HANDLE_NULL and (game.party_alignment == CHAOTIC_NEUTRAL or game.party_alignment == CHAOTIC_GOOD or game.party_alignment == CHAOTIC_EVIL)):
				attachee.cast_spell(spell_magic_circle_against_chaos, attachee)
				attachee.spells_pending_to_memorized()
#				return RUN_DEFAULT
	for obj in game.party[0].group_list():
		if (attachee.distance_to(obj) <= 10 and game.global_vars[726] == 5):
			if (game.party_alignment == NEUTRAL_GOOD and attachee.leader_get() == OBJ_HANDLE_NULL):
				attachee.cast_spell(spell_magic_circle_against_good, attachee)
				attachee.spells_pending_to_memorized()
#				return RUN_DEFAULT
	for obj in game.party[0].group_list():
		if (attachee.distance_to(obj) <= 10 and game.global_vars[726] == 5):
			if (game.party_alignment == NEUTRAL_EVIL and attachee.leader_get() == OBJ_HANDLE_NULL):
				attachee.cast_spell(spell_magic_circle_against_evil, attachee)
				attachee.spells_pending_to_memorized()
#				return RUN_DEFAULT	
	if (game.global_vars[726] >= 200):
		game.global_vars[726] = 0
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	if ( game.story_state <= 1 and game.global_flags[806] == 0):
		game.story_state = 2	
	if ( game.global_flags[340] == 1 and game.global_flags[806] == 0):
		# Playing demo version
		game.global_flags[834] = 1 
		triggerer.begin_dialog( attachee, 380 )

	# Make his surviving fellows disappear
	for obj in game.obj_list_vicinity( attachee.location, OLC_NPC):
		if obj.name in ( range(14074, 14078) ) and obj.stat_level_get( stat_hp_current ) >= 0 and obj.leader_get() == OBJ_HANDLE_NULL:
			obj.runoff(obj.location - 3)
			game.global_vars[756] += 1
			game.timevent_add(destroy, (obj), 1000 + game.random_range(0, 200), 1)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	if (game.global_flags[806] == 0):
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
			attachee.reaction_set( pc, 50 )
		for npc in game.obj_list_vicinity(attachee.location,OLC_NPC):
			for pc in game.party:
				npc.ai_shitlist_remove( pc )
				npc.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	randy1 = game.random_range(1,12)		# added by ShiningTed
#	if (attachee.map == 5065):				## Removed by Livonya
#		leader = attachee.leader_get()		## Removed by Livonya
#		if (leader != OBJ_HANDLE_NULL):		## Removed by Livonya
#			leader.begin_dialog( attachee, 270 )## Removed by Livonya
	if ((attachee.map == 5111) and ( game.global_flags[200] == 0 )):
		game.global_flags[200] = 1
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 290 )
	if (attachee.map == 5066):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 300 )
	if (game.global_flags[833] == 0 and (attachee.map == 5001 or attachee.map == 5051)):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 600 )
	if ((attachee.map == 5013) and randy1 >= 6):	# added by ShiningTed
		attachee.float_line(12029,triggerer)
	if ((attachee.map == 5042) and randy1 >= 8):	# added by ShiningTed
		attachee.float_line(12060,triggerer)
	return RUN_DEFAULT


def buff_npc( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14424 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.turn_towards(attachee)
			obj.cast_spell(spell_mage_armor, obj)
		if (obj.name == 14425 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.turn_towards(attachee)
			obj.cast_spell(spell_shield_of_faith, obj)
	attachee.cast_spell(spell_shield_of_faith, attachee)
	return RUN_DEFAULT


def buff_npc_two( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14424 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_mirror_image, obj)
		if (obj.name == 14425 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_owls_wisdom, obj)
	attachee.cast_spell(spell_bulls_strength, attachee)
	return RUN_DEFAULT


def buff_npc_three( attachee, triggerer ):
	if (not attachee.has_wielded(1) or not attachee.has_wielded(4071)):
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	record_time_stamp(425)
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	attachee.runoff(attachee.location-3)

	obj_list = game.obj_list_vicinity( attachee.location, OLC_NPC)
	for obj in game.obj_list_vicinity( attachee.location - 35, OLC_NPC):
		if not (obj in obj_list):
			obj_list += (obj,)
	for obj in game.obj_list_vicinity( attachee.location + 35, OLC_NPC):
		if not (obj in obj_list):
			obj_list += (obj,)
	for obj in game.obj_list_vicinity( attachee.location + 60, OLC_NPC):
		if not (obj in obj_list):
			obj_list += (obj,)
	for obj in obj_list:
		if obj.name in ( range(14074, 14078) ) and obj.stat_level_get( stat_hp_current ) >= 0 and obj.leader_get() == OBJ_HANDLE_NULL:
			obj.runoff(obj.location - 3)
			game.global_vars[756] += 1
			game.timevent_add(destroy, (obj), 1000 + game.random_range(0, 200), 1)
#	if game.global_flags[277] == 0:
##Raimol rats the party out to the traders
#		for obj in game.party:
#			if obj.name == 8050:
#				a = game.encounter_queue
#				b = 1
#				for enc_id in a:
#					if enc_id == 3000:
#						b = 0
#				if b == 1:
#					game.global_flags[420] = 1
#					game.encounter_queue.append(3000)
	return RUN_DEFAULT


def	demo_end_game( attachee, triggerer ):
	# play slides and end game
	return RUN_DEFAULT


def argue_ron( attachee, triggerer, line): # added by ShiningTed
	npc = find_npc_near(attachee,14681)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(triggerer)
		triggerer.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,260)
	return SKIP_DEFAULT


def equip_transfer( attachee, triggerer ):
	itemA = attachee.item_find(4071)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.destroy()
		create_item_in_inventory( 4071, triggerer )
	create_item_in_inventory( 7001, attachee )
	return RUN_DEFAULT


def create_spiders( attachee, triggerer ): # added by ShiningTed

	# Modified by Sitra Achara - 
	# Checks no. of troop casualties
	# If less than 5 - do not spawn spiders
	ggv401 = game.global_vars[401]
	troop_casualties = ((ggv401 >> 3) & 15) + ((ggv401 >> 7) & 15)
	if troop_casualties < 5:
		return

	spider1 = game.obj_create( 14397, location_from_axis (474L, 535L) )
	game.particles( "sp-summon monster I", spider1 )
	game.sound( 4033, 1 )
	spider1.turn_towards(game.party[0])
	spider2 = game.obj_create( 14398, location_from_axis (481L, 536L) )
	game.particles( "sp-summon monster I", spider2 )
	spider2.turn_towards(game.party[0])
	spider3 = game.obj_create( 14398, location_from_axis (470L, 536L) )
	game.particles( "sp-summon monster I", spider3 )
	spider3.turn_towards(game.party[0])
	spider4 = game.obj_create( 14417, location_from_axis (529L, 544L) )
	spider5 = game.obj_create( 14417, location_from_axis (532L, 555L) )
	return RUN_DEFAULT


def lareth_troops_state():
	seleucas = OBJ_HANDLE_NULL
	lareth_sarge_1 = OBJ_HANDLE_NULL
	lareth_sarge_2 = OBJ_HANDLE_NULL
	troop_count = 0
	for obj in game.obj_list_vicinity(location_from_axis (490, 535), OLC_NPC):
		if obj.name == 14074:
			if obj.is_unconscious() == 0:
				troop_count += 1
			else:
				curr = obj.stat_level_get( stat_hp_current )
				maxx = obj.stat_level_get( stat_hp_max )
		elif obj.name == 14077:
			seleucas = obj
		elif obj.name == 14075:
			lareth_sarge_1 = obj
		elif obj.name == 14076:
			lareth_sarge_2 = obj
	return 0


def destroy(obj): # Destroys object.  Neccessary for time event destruction to work.
	obj.destroy()
	return 1
	

def lareth_abandon( attachee, triggerer ):
	game.leader.follower_remove( attachee )
	return
	
	
def party_too_far_from_lareth( lareth ):
	for pc in game.leader.group_list():
		xx = pc.location & ( 0xFFFF )
		if xx <= 495 or lareth.distance_to(pc) <= 42:
			return 0
	return 1


def player_entrenched_in_corridor( attachee ):
	outside_corridor_count = 0
	pc_count = 0
	for pc in game.party:
		pc_count += 1
		xx = pc.location & ( 0xFFFF )
		yy = pc.location >> 32
		if xx < 495 or yy < 546:
			outside_corridor_count += 1
			return 0
	if outside_corridor_count >= 2:
		return 0
	elif pc_count <= 2 and outside_corridor_count == 1:
		return 0
	else:
		return 1

	
def kill_beacon_obj( obj_beacon, attachee ):
	if attachee.d20_query(Q_Prone) == 1:
		obj_beacon.destroy()
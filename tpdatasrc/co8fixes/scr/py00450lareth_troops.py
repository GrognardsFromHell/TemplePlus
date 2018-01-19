from toee import *
from utilities import *
from py00439script_daemon import *
from Livonya import *
from combat_standard_routines import *


###
### IMPORTANT NOTICE!
### I have directly copied many scripts instead of using functions.
### This is because when you call another script, it often fails to execute.
### Thus I wished to cut back on them as much as possible.



## Flags / Vars documentation:
##
# ggv400: Bitwise flags
# 2**0 - Seleucas engaged in battle
# 2**1 - Seleucas dead
# 2**2 - Lareth has been called upon by his men
# 2**3 - Lareth has entered the fray
# 2**4 - Abort Lareth's villain speech
# 2**5 - Entered combat with Lareth without having entered combat with Seleucas

# ggv401: Mini variables
# 0-2: Lareth Float Comment Stage
# 3-6: Seleucas Battle Death count
# 7-10: Seleucas Battle Knockout count
# 11-14: Seleucas Battle Charm count

# ggv403: Moathouse  Reactive behavior flags
# 0 - Moathouse regrouped; 
# 1 - New guard guy shouts out to Seleucas - enables his KOS; 
# 2 - KOS thing played out


# pad3:
# 2**1 - marked as having contributed to Seleucas Battle Knockout count
# 2**2 - marked as having contributed to Seleucas Battle Charm count
# 2**5 - marked as having contributed to Seleucas Battle Death count
# 2**6 - solo guard who went to alert Seleucas
# 2**7 - have had Sleep successfully cast upon (see Spell438 - Sleep.py)
# 2**8 - Beacon designator
# 2**9 - Archer guy put to sleep - skip turn when woken up to prevent approach, then reset


def san_dialog( attachee, triggerer ):
	if (game.global_flags[37] == 1 and (game.global_flags[49] == 1 or game.global_flags[48] == 0)):
		triggerer.begin_dialog( attachee, 40 )
	elif (game.global_flags[49] == 1):
		triggerer.begin_dialog( attachee, 60 )
	elif (game.global_flags[48] == 1):
		triggerer.begin_dialog( attachee, 50 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer, generated_from_timed_event_call = 0,  kill_count = -1  ):
	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 12, 1 ) 

	ggv400 = game.global_vars[400]
	ggv401 = game.global_vars[401]
	pad3 = attachee.obj_get_int(obj_f_npc_pad_i_3)

	if generated_from_timed_event_call == 0:
		game.global_vars[756] += 1
		game.timevent_add(san_dying, (attachee, triggerer, 1), 100+ game.random_range(0, 50), 1)

	if ggv400 & (2**0) != 0: # Fighting Seleucas
		if attachee.is_unconscious() == 1 and (pad3 & (2**5)) == 0:
			pad3 += (2**5)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3 ) # Mark him as having contributed to the dying count
			ggv401 += (1 << 3)
			game.global_vars[401] = ggv401

			# Was this guy unconscious before dying? 
			# (thus having contributed to the unconscious count)
			# Unmark his internal flag and reduce the count of unconscious guys

		if (pad3 & (2**1)) != 0:
			pad3 ^= (2**1)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3 )
			if (ggv401 >> 7) & 15 > 0:
				ggv401 -= (1<<7)
			game.global_vars[401] = ggv401
	if attachee.name == 14077:
		# Seleucas dead
		ggv400 |= 2**1
		game.global_vars[400] = ggv400

	return RUN_DEFAULT




def san_enter_combat(attachee, triggerer):
	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 13, 1 ) 

	if attachee.name == 14077: # Seleucas
		ggv400 = game.global_vars[400]
		ggv403 = game.global_vars[403]

		ggv400 = ggv400 | (2**0) # indicate that you have entered combat with Seleucas' group
		game.global_vars[400] = ggv400
		if  (ggv403 & (2**1) ) != 0: # Moathouse regroup scenario; Seleucas is warned by guard of approaching party
			ggv403 |= 2**2
			game.global_vars[403] = ggv403
		attachee.scripts[19] = 0 # His heartbeat script - disabled, in case hostilies momentarily cease / fight ends with him charmed
		if game.global_flags[403] == 1 and game.global_flags[405] == 1:
			game.leader.float_mesfile_line( 'mes\\script_activated.mes', 10001, 1 ) 
	elif attachee.name == 14074:
		pad3 = attachee.obj_get_int(obj_f_npc_pad_i_3)
		if ( pad3 & (2**6)) != 0:
			attachee.float_line(15002, triggerer)
			attachee.scripts[9] = 0
			pad3 ^= 2**6
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)


	a123 = attachee.item_worn_at(3).obj_get_int(obj_f_weapon_type) 
	if a123 in [14, 17, 46, 48, 68]:
		# 14 - light crossbow
		# 17 - heavy crossbow
		# 46 - shortbow
		# 48 - longbow
		# 68 - repeating crossbow
		dummy = 1
	else:
		tag_strategy(attachee)
		if a123 in [37, 41, 43, 44, 61]: 
			# Reach Weapons
			get_melee_reach_strategy(attachee)
		else:
			get_melee_strategy(attachee)




def san_exit_combat(attachee, triggerer):
	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 14, 1 ) 
	return RUN_DEFAULT




def san_start_combat( attachee, triggerer, generated_from_timed_event_call = 0 ): 

	if attachee.stat_base_get( stat_dexterity ) == -30:
		# Beacon object - skip its turn in case it somehow survives
		attachee.destroy()
		return SKIP_DEFAULT
		

	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 )

	ggv400 = game.global_vars[400]
	ggv401 = game.global_vars[401]
	pad3 = attachee.obj_get_int(obj_f_npc_pad_i_3)
	
	
	if ggv400 & (2**0) != 0:
		if attachee.is_unconscious() == 1 and (pad3 & (2**1)) == 0: 
			# Troop knocked unconscious - mark him as such and increase the KOed counter
			pad3 |= 2**1
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)

			ggv401 += 1<<7
			game.global_vars[401] = ggv401
		elif (pad3 & (2**1)) != 0 and attachee.is_unconscious() == 0:
			# Attachee has contributed to unconscious count, but is no longer unconscious
			# E.g. healed
			pad3 &= ~(2**1)
			if ( (ggv401 >> 7) & 15 ) > 0:
				ggv401 -= (1 << 7)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
			game.global_vars[401] = ggv401
			
		
		if attachee.name in [14074, 14075, 14076, 14077]:
			# The "Call Lareth" section
			# Calculates how many troops are down etc. and decides whether to call Lareth
			seleucas = OBJ_HANDLE_NULL
			lareth_sarge = OBJ_HANDLE_NULL
			for obj1 in game.obj_list_vicinity(attachee.location, OLC_NPC):
				if obj1.name == 14077:
					seleucas = obj1
				elif obj1.name == 14076:
					lareth_sarge = obj1
			if (seleucas != OBJ_HANDLE_NULL and seleucas.is_unconscious() == 1) or (ggv400 & (2**1)) != 0 or ( seleucas.leader_get() != OBJ_HANDLE_NULL ):
				seleucas_down = 1
			else:
				seleucas_down = 0
			if (lareth_sarge != OBJ_HANDLE_NULL and lareth_sarge.is_unconscious() == 1 ):
				lareth_sarge_down = 1
			else:
				lareth_sarge_down = 0

			troops_down = ((ggv401 >> 3) & 15) + ((ggv401 >> 7) & 15)
	
			should_call_lareth = 0
			if seleucas_down and (troops_down >= 2): # Seleucas + 1 other troop
				should_call_lareth = 1
			elif seleucas_down and lareth_sarge_down: # Seleucas + Sergeant
				should_call_lareth = 1
			elif troops_down >= 2:
				should_call_lareth = 1
	
			if should_call_lareth:
				if (ggv400 & ( (2**2) + (2**5) )) == 0:
					# Lareth has not been called upon
					# And have not initiated combat with Lareth
					if seleucas_down and attachee.is_unconscious() == 0 and ( attachee.leader_get() == OBJ_HANDLE_NULL ):
						# Seleucas is down - the soldier calls Lareth
						temppp = attachee.scripts[9]
						attachee.scripts[9] = 450
						attachee.float_line(15001, triggerer)
						attachee.scripts[9] = temppp
						ggv400 |= 2**2
						game.global_vars[400] = ggv400
					elif seleucas_down == 0:
						# Seleucas is alive - Seleucas calls Lareth
						seleucas.float_line(15000, triggerer)
						ggv400 |= 2**2
						game.global_vars[400] = ggv400
				elif (ggv400 & ( (2**3) + (2**5) ) ) == 0:
					lareth = find_npc_near(attachee, 8002)
					if lareth != OBJ_HANDLE_NULL and lareth.is_unconscious() == 0:
						#if can_see_party(lareth) == 0:
						lareth.obj_set_int(obj_f_critter_strategy, 81) # Lareth's "Join the Fray" strategy - start with shield of faith, then target hurt friendly, heal, and commence attack
						lareth.scripts[16] = 60 # san_end_combat
						lareth.move( location_from_axis(476, 546), -60, 10 ) # This is to decrease the chances of Lareth skipping his turn
						closest_party_member = game.leader
						for pcc in game.party:
							if pcc.distance_to(attachee) < closest_party_member.distance_to(attachee):
								closest_party_member = pcc
						lareth.cast_spell( spell_shield_of_faith, lareth )
						lareth.attack( closest_party_member )
						init_current = -100
						for pc in game.party:
							if pc.get_initiative() < attachee.get_initiative() and ( pc.get_initiative() - 2 ) > init_current:
								init_current = pc.get_initiative() - 2
						lareth.set_initiative( init_current ) # make sure he gets to act on the same round
						ggv400 |= 2**3
						game.global_vars[400] = ggv400
				

		if attachee.leader_get() != OBJ_HANDLE_NULL and (pad3 & (2**2)) == 0:
			# Attachee charmed
			pad3 |= 2**2
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
			ggv401 += (1<<11)
			game.global_vars[401] = ggv401
		
	#################################
	# Copied from script 310 :	#
	#################################
	## THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
		return RUN_DEFAULT
	#	create_item_in_inventory( 8903, attachee )

	##	attachee.d20_send_signal(S_BreakFree)
	
	
	#################################
	# Wake up from Sleep Scripting	#
	#################################
	'''

	if generated_from_timed_event_call == 0 and attachee.is_unconscious() == 0 and attachee.d20_query(Q_Prone) == 0 and attachee.leader_get() == OBJ_HANDLE_NULL:
		# checks if attachee is capable of waking someone up (i.e. not prone, KOed or charmed)
		for obj in game.obj_list_cone(attachee, OLC_NPC, 10, 0, 360):
			if obj.name in range(14074, 14078) and obj.distance_to(attachee) <= 4.2 and obj != attachee:
				# Scan all NPCs in arm's reach
				obj_pad3 = obj.obj_get_int(obj_f_npc_pad_i_3)
				if obj_pad3 & (2**7) != 0:
					# NPC was victim of "Sleep" spell
					obj_pad3 ^= 2**7
					obj.obj_set_int(obj_f_npc_pad_i_3, obj_pad3)
					if attachee.scripts[9] == 0:
						attachee.scripts[9] = 450 ##Set the correct san_dialog
					attachee.turn_towards(obj)
					attachee.float_line( 16500 + game.random_range(0, 3), attachee )
					obj.damage(OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new('1d1') )
					#obj_pad3 ^= 2**9 #marks obj as having just been woken up
					#obj.obj_set_int(obj_f_npc_pad_i_3, obj_pad3)
					return SKIP_DEFAULT
			
	if (pad3 & (2**9) != 0) and attachee.is_unconscious() == 0:
		# Restore woken up archer's strategey
		# Note: woken up critters don't fire their san_start_combat script on the turn they get up
		pad3 ^= 2**9
		attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
		attachee.obj_set_int(obj_f_critter_strategy, attachee.obj_get_int(obj_f_pad_i_0))

	'''			
	a123 = attachee.item_worn_at(3).obj_get_int(obj_f_weapon_type) 
	if a123 in [14, 17, 46,  48, 68]: # (Is archer)
		# 14 - light crossbow
		# 17 - heavy crossbow
		# 46 - shortbow
		# 48 - longbow
		# 68 - repeating crossbow
		


		
		if generated_from_timed_event_call == 0 and attachee.is_unconscious() == 0 and attachee.d20_query(Q_Prone) == 0 and attachee.leader_get() == OBJ_HANDLE_NULL:
		
			player_in_obscuring_mist = 0
			grease_detected = 0
			should_wake_up_comrade = 0
			
			# First, find closest party member - the most likely target for an archer
			closest_pc_1 = game.leader
			for pc in game.party[0].group_list():
				if pc.distance_to(attachee) < closest_pc_1.distance_to(attachee):
					closest_pc_1 = pc
					
			
			for spell_obj in game.obj_list_cone(closest_pc_1, OLC_GENERIC, 30, 0, 360):
				# Check for active OBSCURING MIST spell objects			
				if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 333 + (1<<15) and spell_obj.distance_to(closest_pc_1) <= 17.5:
					player_in_obscuring_mist = 1
					
			for spell_obj in game.obj_list_cone(closest_pc_1, OLC_GENERIC, 40, 0, 360):
				# Check for active GREASE spell object
				if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 200 + (1<<15):
					grease_detected = 1
					grease_obj = spell_obj
					
		### Scripting for approaching sleeping friend
			if player_in_obscuring_mist == 1 or game.random_range(0,1) == 0:
				# Player staying back in Obscuring Mist - thus it's more useful to use the archer to wake someone up
				# Otherwise, 50% chance
				should_wake_up_comrade = 1
			if closest_pc_1.distance_to(attachee) <= 8: 
				# Player is close - Attachee will probably get hit trying to approach friend, so abort
				should_wake_up_comrade = 0
				
			'''
			if should_wake_up_comrade == 1:
				for obj in game.obj_list_cone(attachee, OLC_NPC, 45, 0, 360):
					if obj.distance_to(attachee) <= 17 and  (   (obj.obj_get_int(obj_f_npc_pad_i_3) & 2**7)  !=  0   ):
						# Draw a line between attachee and obj (the wake-up target)
						# If a PC is detected inbetween, abort!
						x0, y0 = location_to_axis(attachee.location)
						x1, y1 = location_to_axis(obj.location)
						someone_elses_problem = 0
						for qq in range(0, 26):
							xs = int( x0 + ( (qq * (x1-x0))/25 ) )
							ys = int( y0 + ( (qq * (y1-y0))/25 ) )
							if grease_detected:				
								dummy =1
								gx,gy = location_to_axis(grease_obj.location)
								if abs(xs - gx) + abs(ys - gy) <= 9:
									someone_elses_problem = 1
							for pc in game.party:
								pcx, pcy = location_to_axis(pc.location)
								if abs(pcx-xs) + abs(pcy-ys) <= 2:
									someone_elses_problem = 1
						if someone_elses_problem == 0:
							obj_beacon = game.obj_create(14074, obj.location)
							obj_beacon.object_flag_set(OF_DONTDRAW)
							obj_beacon.object_flag_set(OF_CLICK_THROUGH)
							obj_beacon.move(obj.location,0,0)
							obj_beacon.stat_base_set( stat_dexterity, -30 )
							obj_beacon.obj_set_int(obj_f_npc_pad_i_3, 2**8 )
							
							attachee.obj_set_int(obj_f_pad_i_0, attachee.obj_get_int(obj_f_critter_strategy) ) # Record original strategy
							attachee.obj_set_int(obj_f_critter_strategy, 84) # "Seek low AC ally" strat
							if attachee.scripts[9] == 0:
								attachee.scripts[9] = 450
							if game.random_range(0, 5) == 0:
								if attachee.scripts[9] == 0:
									attachee.scripts[9] = 450
								attachee.float_line( 16510 + game.random_range(0,1), triggerer)
							return RUN_DEFAULT
						
			'''
			if player_in_obscuring_mist == 1 and game.random_range(1,12) == 1:
				# Player Cast Obscuring Mist - 
				# Float a comment about not being able to see the player (1 in 12 chance)
				if attachee.scripts[9] == 0:
					attachee.scripts[9] = 450
				attachee.float_line(16520 + game.random_range(0,2), attachee)


					
	else:
		tag_strategy(attachee)
		if a123 in [37, 41, 43, 44, 61]: 
			# Reach Weapons
			get_melee_reach_strategy(attachee)
		else:
			game.global_vars[499] = 19
			get_melee_strategy(attachee)

	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	


def san_end_combat( attachee, triggerer, generated_from_timed_event_call = 0 ): 

	ggv400 = game.global_vars[400]
	ggv401 = game.global_vars[401]
	pad3 = attachee.obj_get_int(obj_f_npc_pad_i_3)

	
	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 16, 1 ) 


	if ggv400 & (2**0) != 0:
		if attachee.is_unconscious() == 1 and (pad3 & (2**1)) == 0:
			pad3 |= 2**1
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)

			ggv401 += 1<<7
			game.global_vars[401] = ggv401
		elif (pad3 & (2**1)) != 0 and attachee.is_unconscious() == 0:
			# Attachee has contributed to unconscious count, but is no longer unconscious
			# E.g. healed
			pad3 &= ~(2**1)
			if ( (ggv401 >> 7) & 15 ) > 0:
				ggv401 -= (1 << 7)
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
			game.global_vars[401] = ggv401
			
		
		if attachee.name in [14074, 14075, 14076, 14077]:
			seleucas = OBJ_HANDLE_NULL
			lareth_sarge = OBJ_HANDLE_NULL
			for obj1 in game.obj_list_vicinity(attachee.location, OLC_NPC):
				if obj1.name == 14077:
					seleucas = obj1
				elif obj1.name == 14076:
					lareth_sarge = obj1
			if (seleucas != OBJ_HANDLE_NULL and seleucas.is_unconscious() == 1) or (ggv400 & (2**1)) != 0 or seleucas.leader_get() != OBJ_HANDLE_NULL:
				seleucas_down = 1
			else:
				seleucas_down = 0
			if (lareth_sarge != OBJ_HANDLE_NULL and lareth_sarge.is_unconscious() == 1 ):
				lareth_sarge_down = 1
			else:
				lareth_sarge_down = 0

			troops_down = ((ggv401 >> 3) & 15) + ((ggv401 >> 7) & 15)
	
			should_call_lareth = 0
			if seleucas_down and (troops_down >= 2): # Seleucas + 1 other troop
				should_call_lareth = 1
			elif seleucas_down and lareth_sarge_down: # Seleucas + Sergeant
				should_call_lareth = 1
			elif troops_down >= 2:
				should_call_lareth = 1
	
			if should_call_lareth:
				if (ggv400 & ( (2**2) + (2**5) )) == 0:
					# Lareth has not been called upon
					# And have not initiated combat with Lareth
					if seleucas_down and attachee.is_unconscious() == 0 and attachee.leader_get() == OBH_HANDLE_NULL:
						temppp = attachee.scripts[9]
						attachee.scripts[9] = 450
						attachee.float_line(15001, triggerer)
						attachee.scripts[9] = temppp
						ggv400 |= 2**2
						game.global_vars[400] = ggv400
					elif seleucas_down == 0:
						seleucas.float_line(15000, triggerer)
						ggv400 |= 2**2
						game.global_vars[400] = ggv400
				elif (ggv400 & (2**3)) == 0:
					lareth = find_npc_near(attachee, 8002)
					if lareth != OBJ_HANDLE_NULL and lareth.is_unconscious() == 0:
						#if can_see_party(lareth) == 0:
						lareth.obj_set_int(obj_f_critter_strategy, 81) # Lareth's "Join the Fray" strategy - start with shield of faith, then target hurt friendly, heal, and commence attack
						lareth.scripts[16] = 60 # san_end_combat
						lareth.move( location_from_axis(476, 546), -60, 10 ) # This is to decrease the chances of Lareth skipping his turn
						closest_party_member = game.leader
						for pcc in game.party:
							if pcc.distance_to(attachee) < closest_party_member.distance_to(attachee):
								closest_party_member = pcc
						lareth.cast_spell( spell_shield_of_faith, lareth )
						lareth.attack( closest_party_member )
						init_current = -100
						for pc in game.party:
							if pc.get_initiative() < attachee.get_initiative() and ( pc.get_initiative() - 2 ) > init_current:
								init_current = pc.get_initiative() - 2
						lareth.set_initiative( init_current ) # make sure he gets to act on the same round
						ggv400 |= 2**3
						game.global_vars[400] = ggv400
				

		if attachee.leader_get() != OBJ_HANDLE_NULL and (pad3 & (2**2)) == 0:
			# Attachee charmed
			pad3 |= 2**2
			attachee.obj_set_int(obj_f_npc_pad_i_3, pad3)
			ggv401 += (1<<11)
			game.global_vars[401] = ggv401

	
	if generated_from_timed_event_call == 0 and attachee.is_unconscious() == 0 and attachee.d20_query(Q_Prone) == 0:
		# Wake up sleeping guy script
		bbb = attachee.obj_get_int(obj_f_critter_strategy)
		if bbb == 84: # if using the 'seek beacon' strategy
			bbb = attachee.obj_get_int(obj_f_pad_i_0) # retrieve previous strat
			attachee.obj_set_int(obj_f_critter_strategy, bbb)
			has_woken_someone_up = 0
			for obj in game.obj_list_cone(attachee, OLC_NPC, 10, 0, 360):
				if obj.name in range(14074, 14078) and obj != attachee:
					obj_pad3 = obj.obj_get_int(obj_f_npc_pad_i_3)
					if obj_pad3 & (2**8) != 0: # is a beacon object
						obj.destroy()
					elif obj_pad3 & (2**7) != 0 and obj.distance_to(attachee) <= 4.2 and has_woken_someone_up == 0:
						obj_pad3 &= ~(2**7) # remove "sleepig flag"
						obj.obj_set_int(obj_f_npc_pad_i_3, obj_pad3)
						if attachee.scripts[9] == 0:
							attachee.scripts[9] = 450 ##Set the correct san_dialog
						attachee.turn_towards(obj)
						attachee.float_line( 16500 + game.random_range(0, 3), attachee )
						obj.damage(OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new('1d1') )
						has_woken_someone_up = 1
						


def san_heartbeat(attachee, triggerer):
	if (not game.combat_is_active()) and ( game.global_flags[363] == 0 ) and (attachee.leader_get() == OBJ_HANDLE_NULL):
		if ( attachee.distance_to(game.party[0]) <= 22 and attachee.can_see(game.party[0]) and not critter_is_unconscious(game.party[0]) ): 
			if (not attachee.has_met(game.party[0])):	
				attachee.turn_towards(game.party[0])
				game.party[0].begin_dialog( attachee, 1 )
				game.new_sid = 0 				
			elif (game.global_flags[49] == 0 and game.global_flags[48] == 1 and game.global_flags[62] == 1):
				attachee.turn_towards(game.party[0])
				game.party[0].begin_dialog( attachee, 50 )
				game.new_sid = 0
			elif (game.global_flags[49] == 1):
				attachee.turn_towards(game.party[0])
				game.party[0].begin_dialog( attachee, 60 )
				game.new_sid = 0
			else:
				attachee.turn_towards(game.party[0])
				game.party[0].begin_dialog( attachee, 70 )
				game.new_sid = 0
		else:
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if ( attachee.distance_to(obj) <= 20 and attachee.can_see(obj) and not critter_is_unconscious(obj) ):
					if (not attachee.has_met(obj)):
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 1 )
					elif (game.global_flags[49] == 0 and game.global_flags[48] == 1 and game.global_flags[62] == 1):
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 50 )
					elif (game.global_flags[49] == 1):
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 60 )
					else:
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 70 )
					# Don't scrap the script - because of the "bring the leader here" treatment
	return RUN_DEFAULT


def san_will_kos(attachee, triggerer):
	if attachee.distance_to(triggerer) <= 31 and (game.global_vars[403] & ((2**1) + (2**2)) ) == 2:
		return RUN_DEFAULT
	return SKIP_DEFAULT



def san_spell_cast(attachee, triggerer, spell):
	if spell.spell == spell_fireball:
		game.timevent_add(san_start_combat, (attachee, triggerer, 1), 1250 + game.random_range(0, 1000), 1)
		if game.random_range(0, 5) == 0 and attachee.name == 14074:
			if attachee.scripts[9] == 0:
				attachee.scripts[9] = 450
			if attachee.scripts[9] == 450 and attachee.is_unconscious() == 0 and attachee.d20_query(Q_Prone) == 0:
				attachee.float_line(16000 + game.random_range(0, 6), triggerer)
	else:
		game.timevent_add(san_start_combat, (attachee, triggerer, 1), 4500 + game.random_range(0, 1000), 1)
	return RUN_DEFAULT







###############
###############
###############

def call_leader(npc, pc): 
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 1)
	return 

def run_off( attachee, triggerer ):
	loc = location_from_axis(526,569)
	attachee.runoff(loc)
	return RUN_DEFAULT

def run_off_to_back( attachee, triggerer, generated_from_timevent = 0 ):
	if generated_from_timevent == 0:
		attachee.npc_flag_set(ONF_WAYPOINTS_DAY)
		attachee.npc_flag_set(ONF_WAYPOINTS_NIGHT)
		attachee.runoff(attachee.location - 10 )
		game.global_vars[403] |= 2**1
		obj = game.obj_create( 14074, location_from_axis (485, 538) )
		obj.scripts[9] = 450
		obj.scripts[12] = 450
		obj.scripts[13] = 450
		obj.scripts[14] = 450
		obj.scripts[15] = 450
		obj.scripts[16] = 450
		obj.scripts[41] = 450
		pad3 = obj.obj_get_int(obj_f_npc_pad_i_3)
		pad3 |= 2**6
		obj.obj_set_int(obj_f_npc_pad_i_3, pad3)
		obj.move(location_from_axis(485, 538), 0,0)
		obj.turn_towards(game.leader) # just to correct the animation glitch
		obj.rotation = 3.3
		game.timevent_add(run_off_to_back, (attachee, triggerer, 1), 1000, 1)		
	else:
		attachee.destroy()

	return RUN_DEFAULT


def move_pc( attachee, triggerer ):
	game.fade_and_teleport(0,0,0,5005,537,545)
	# triggerer.move( location_from_axis( 537, 545 ) )
	return RUN_DEFAULT

def deliver_pc( attachee, triggerer ):
	triggerer.move( location_from_axis( 491, 541 ) )
	return RUN_DEFAULT
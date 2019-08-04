from toee import *

from utilities import *
from batch import *
from itt import *

from math import sqrt, atan2

import _include
from co8Util.PersistentData import *


## Contained in this script
TS_CRITTER_KILLED_FIRST_TIME = 504          # KOS monster on Temple Level 1
TS_EARTH_CRITTER_KILLED_FIRST_TIME = 505    # Robe-friendly monster on Temple Level 1
TS_EARTH_TROOP_KILLED_FIRST_TIME = 506      # Earth Temple human troop
TS_CRITTER_THRESHOLD_CROSSED = 509          # Time when you crossed the threshold from killing a monster


#########################################
# Persistent flags/vars/strs		#
# Uses keys starting with		#
# 'Flaggg', 'Varrr', 'Stringgg' 	#
#########################################

def get_f(flagkey):
	flagkey_stringized = 'Flaggg' + str(flagkey)
	tempp = Co8PersistentData.getData(flagkey_stringized)
	if isNone(tempp):
		return 0
	else:
		return int(tempp) != 0

def set_f(flagkey, new_value = 1):
	flagkey_stringized = 'Flaggg' + str(flagkey)
	Co8PersistentData.setData(flagkey_stringized, new_value)

def get_v(varkey):
	varkey_stringized = 'Varrr' + str(varkey)
	tempp = Co8PersistentData.getData(varkey_stringized)
	if isNone(tempp):
		return 0
	else:
		return int(tempp)

def set_v(varkey, new_value):
	varkey_stringized = 'Varrr' + str(varkey)
	Co8PersistentData.setData(varkey_stringized, new_value)
	return get_v(varkey)

def inc_v(varkey, inc_amount = 1):
	varkey_stringized = 'Varrr' + str(varkey)
	Co8PersistentData.setData(varkey_stringized, get_v(varkey) + inc_amount)
	return get_v(varkey)



def get_s(strkey):
	strkey_stringized = 'Stringgg' + str(strkey)
	tempp = Co8PersistentData.getData(strkey_stringized)
	if isNone(tempp):
		return ''
	else:
		return str(tempp)

def set_s(strkey, new_value):
	new_value_stringized = str(new_value)
	strkey_stringized = 'Stringgg' + str(strkey)
	Co8PersistentData.setData(strkey_stringized, new_value_stringized)


#########################################
# Bitwise NPC internal flags			#
# 1-31									#
# Uses obj_f_npc_pad_i_4			 	#
# obj_f_pad_i_3 is sometimes nonzero    #
# pad_i_4, pad_i_5 tested clean on all  #
# protos								#
#########################################

def npc_set(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	tempp = attachee.obj_get_int(obj_f_npc_pad_i_4) | abc
	attachee.obj_set_int(obj_f_npc_pad_i_4, tempp)
	return	

def npc_unset(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	tempp = (attachee.obj_get_int(obj_f_npc_pad_i_4) | abc) - abc
	attachee.obj_set_int(obj_f_npc_pad_i_4, tempp)
	return	

def npc_get(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	return attachee.obj_get_int(obj_f_npc_pad_i_4) & abc != 0




################################################################
################################################################
################################################################
################################################################



def san_dying(attachee, triggerer):
	# in case the 'script bearer' dies, pass the curse to someone else
	not_found = 1
	for pc in game.party:
		if pc.stat_level_get( stat_hp_current ) > 0 and not_found == 1 and pc.type == obj_t_pc:
			not_found = 0
			attachee.scripts[12] = 0
			attachee.scripts[38] = 0
			pc.scripts[12] = 439 #san_dying
			pc.scripts[38] = 439 #san_new_map
			pc.scripts[14] = 439 #san_exit_combat - executes when exiting combat mode
			return

def san_exit_combat( attachee, triggerer ):
	if attachee.map == 5066: # temple level 1
		grate_obj = OBJ_HANDLE_NULL
		for door_candidate in game.obj_list_vicinity( attachee.location, OLC_PORTAL ):
			if (door_candidate.name == 120):
				grate_obj = door_candidate
				
		if not game.combat_is_active():
			harpies_alive = 0
			for obj in game.obj_list_vicinity(attachee.location, OLC_NPC):
				if obj.name == 14243 and obj.leader_get() == OBJ_HANDLE_NULL and obj.is_unconscious() == 0 and obj.stat_level_get( stat_hp_current ) > -10:
					harpies_alive += 1
			if harpies_alive == 0 and (not grate_obj == OBJ_HANDLE_NULL) and game.global_vars[455] & 2**6 == 0:
				game.global_vars[455] |= 2**6
				#grate_obj.object_flag_set(OF_OFF)
				grate_npc = game.obj_create(14913, grate_obj.location)
				grate_npc.move(grate_obj.location, 0, 11.0 )
				grate_npc.rotation = grate_obj.rotation
				#grate_npc.begin_dialog(game.leader, 1000)
	return
		
def san_dialog(attachee, triggerer):
	if (game.leader.map == 5008): # Welcome Wench upstairs - PC left behind
		if (attachee in game.party):
			triggerer.begin_dialog(attachee, 150)
		else:
			triggerer.begin_dialog(attachee, 200)
	return SKIP_DEFAULT
		
def san_new_map( attachee, triggerer ):
	cur_map = attachee.map

	###########################################
	### PC Commentary (float lines/banter)  ###
	###########################################

	if game.party[0].type == obj_t_npc: # leftmost portrait an NPC
		daemon_float_comment(attachee, 1)
		game.timevent_add( daemon_float_comment, (attachee, 2), 5000, 1)
		
	#######################################
	### Global Event Scheduling System  ###
	#######################################

	## Skole Goons
	if tpsts('s_skole_goons', 3*24*60*60) == 1 and get_f('s_skole_goons_scheduled') == 0 and get_f('skole_dead') == 0:
		set_f('s_skole_goons_scheduled')
		if game.quests[42].state != qs_completed and game.global_flags[281] == 0: 
			# ggf281 - have had Skole Goon encounter
			game.quests[42].state = qs_botched
			game.global_flags[202] = 1
			game.encounter_queue.append(3004)
			
	## Thrommel Reward Encounter - 2 weeks
	if tpsts('s_thrommel_reward', 14*24*60*60) == 1 and get_f('s_thrommel_reward_scheduled') == 0:
		set_f('s_thrommel_reward_scheduled')
		if game.global_flags[278] == 0 and not (3001 in game.encounter_queue): 
			# ggf278 - have had Thrommel Reward encounter
			game.encounter_queue.append(3001)

	## Tillahi Reward Encounter - 10 days
	if tpsts('s_tillahi_reward', 10*24*60*60) == 1 and get_f('s_tillahi_reward_scheduled') == 0:
		set_f('s_tillahi_reward_scheduled')
		if game.global_flags[279] == 0 and not (3002 in game.encounter_queue): 
			# ggf279 - have had Tillahi Reward encounter
			game.encounter_queue.append(3002)
			
	## Sargen Reward Encounter - 3 weeks
	if tpsts('s_sargen_reward', 21*24*60*60) == 1 and get_f('s_sargen_reward_scheduled') == 0:
		set_f('s_sargen_reward_scheduled')
		if game.global_flags[280] == 0 and not (3003 in game.encounter_queue): 
			# ggf280 - have had Sargen Reward encounter
			game.encounter_queue.append(3003)

	## Ranth's Bandits Encounter 1 - random amount of days (normal distribution, average of 24 days, stdev = 8 days)
	if tpsts('s_ranths_bandits_1', game.global_vars[923]*24*60*60) == 1 and get_f('s_ranths_bandits_scheduled') == 0:
		set_f('s_ranths_bandits_scheduled')
		if game.global_flags[711] == 0 and not (3434 in game.encounter_queue): 
			# ggf711 - have had Ranth's Bandits Encounter
			game.encounter_queue.append(3434)


	## Scarlet Brotherhood Retaliation for Snitch Encounter - 10 days
	if tpsts('s_sb_retaliation_for_snitch', 10*24*60*60) == 1 and get_f('s_sb_retaliation_for_snitch_scheduled') == 0:
		set_f('s_sb_retaliation_for_snitch_scheduled')
		if game.global_flags[712] == 0 and not (3435 in game.encounter_queue): 
			# ggf712 - have had Scarlet Brotherhood Encounter
			game.encounter_queue.append(3435)

	## Scarlet Brotherhood Retaliation for Narc Encounter - 6 days
	if tpsts('s_sb_retaliation_for_narc', 6*24*60*60) == 1 and get_f('s_sb_retaliation_for_narc_scheduled') == 0:
		set_f('s_sb_retaliation_for_narc_scheduled')
		if game.global_flags[712] == 0 and not (3435 in game.encounter_queue): 
			# ggf712 - have had Scarlet Brotherhood Encounter
			game.encounter_queue.append(3435)

	## Scarlet Brotherhood Retaliation for Whistelblower Encounter - 14 days
	if tpsts('s_sb_retaliation_for_whistleblower', 14*24*60*60) == 1 and get_f('s_sb_retaliation_for_whistleblower_scheduled') == 0:
		set_f('s_sb_retaliation_for_whistleblower_scheduled')
		if game.global_flags[712] == 0 and not (3435 in game.encounter_queue): 
			# ggf712 - have had Scarlet Brotherhood Encounter
			game.encounter_queue.append(3435)

	## Gremlich Scream Encounter 1 - 1 day
	if tpsts('s_gremlich_1', 1*24*60*60) == 1 and get_f('s_gremlich_1_scheduled') == 0:
		set_f('s_gremlich_1_scheduled')
		if game.global_flags[713] == 0 and not (3436 in game.encounter_queue): 
			# ggf713 - have had Gremlich Scream Encounter 1
			game.encounter_queue.append(3436)

	## Gremlich Reset Encounter - 5 days
	if tpsts('s_gremlich_2', 5*24*60*60) == 1 and get_f('s_gremlich_2_scheduled') == 0:
		set_f('s_gremlich_2_scheduled')
		if game.global_flags[717] == 0 and not (3440 in game.encounter_queue): 
			# ggf717 - have had Gremlich Reset Encounter
			game.encounter_queue.append(3440)

	## Mona Sport Encounter 1 (pirates vs. brigands) - 3 days
	if tpsts('s_sport_1', 3*24*60*60) == 1 and get_f('s_sport_1_scheduled') == 0:
		set_f('s_sport_1_scheduled')
		if game.global_flags[718] == 0 and not (3441 in game.encounter_queue): 
			# ggf718 - have had Mona Sport Encounter 1
			game.encounter_queue.append(3441)

	## Mona Sport Encounter 2 (bugbears vs. orcs melee) - 3 days
	if tpsts('s_sport_2', 3*24*60*60) == 1 and get_f('s_sport_2_scheduled') == 0:
		set_f('s_sport_2_scheduled')
		if game.global_flags[719] == 0 and not (3442 in game.encounter_queue): 
			# ggf719 - have had Mona Sport Encounter 2
			game.encounter_queue.append(3442)

	## Mona Sport Encounter 3 (bugbears vs. orcs ranged) - 3 days
	if tpsts('s_sport_3', 3*24*60*60) == 1 and get_f('s_sport_3_scheduled') == 0:
		set_f('s_sport_3_scheduled')
		if game.global_flags[720] == 0 and not (3443 in game.encounter_queue): 
			# ggf720 - have had Mona Sport Encounter 3
			game.encounter_queue.append(3443)

	## Mona Sport Encounter 4 (hill giants vs. ettins) - 3 days
	if tpsts('s_sport_4', 3*24*60*60) == 1 and get_f('s_sport_4_scheduled') == 0:
		set_f('s_sport_4_scheduled')
		if game.global_flags[721] == 0 and not (3444 in game.encounter_queue): 
			# ggf721 - have had Mona Sport Encounter 4
			game.encounter_queue.append(3444)

	## Mona Sport Encounter 5 (female vs. male bugbears) - 3 days
	if tpsts('s_sport_5', 3*24*60*60) == 1 and get_f('s_sport_5_scheduled') == 0:
		set_f('s_sport_5_scheduled')
		if game.global_flags[722] == 0 and not (3445 in game.encounter_queue): 
			# ggf722 - have had Mona Sport Encounter 5
			game.encounter_queue.append(3445)

	## Mona Sport Encounter 6 (zombies vs. lacedons) - 3 days
	if tpsts('s_sport_6', 3*24*60*60) == 1 and get_f('s_sport_6_scheduled') == 0:
		set_f('s_sport_6_scheduled')
		if game.global_flags[723] == 0 and not (3446 in game.encounter_queue): 
			# ggf723 - have had Mona Sport Encounter 6
			game.encounter_queue.append(3446)

	## Bethany Encounter - 2 days
	if tpsts('s_bethany', 2*24*60*60) == 1 and get_f('s_bethany_scheduled') == 0:
		set_f('s_bethany_scheduled')
		if game.global_flags[724] == 0 and not (3447 in game.encounter_queue): 
			# ggf724 - have had Bethany Encounter
			game.encounter_queue.append(3447)

	if tpsts('s_zuggtmoy_banishment_initiate', 4*24*60*60) == 1 and get_f('s_zuggtmoy_gone') == 0 and game.global_flags[326] == 1 and attachee.map != 5016 and attachee.map != 5019:
		set_f('s_zuggtmoy_gone')
		import py00262burne_apprentice
		py00262burne_apprentice.return_Zuggtmoy( game.leader, game.leader )



	##############################################
	### End of Global Event Scheduling System  ###
	##############################################
	if game.global_vars[449] & (2**0 + 2**1 + 2**2) != 0: # If set preference for speed
		speedup(game.global_vars[449] & (2**0 + 2**1 + 2**2) , game.global_vars[449] & (2**0 + 2**1 + 2**2) )
	if game.global_flags[403] == 1: # Test mode enabled; autokill critters!
		#game.particles( "sp-summon monster I", game.leader)

		#game.timevent_add( autokill, (cur_map, 1), 150 )
		autokill(cur_map, autoloot = 1)
		for pc in game.party:
			pc.identify_all()
	if (cur_map == 5004): # Moathouse Upper floor
		if game.global_vars[455] & 2**7 != 0: # Secret Door Reveal
			for obj in game.obj_list_vicinity( lfa(464, 470), OLC_PORTAL | OLC_SCENERY ):
				if obj.obj_get_int( obj_f_secretdoor_flags ) & 2**16 != 0: # OSDF_SECRET_DOOR
					obj.obj_set_int( obj_f_secretdoor_flags, obj.obj_get_int( obj_f_secretdoor_flags ) | 2**17 )
	elif (cur_map == 5005):
		## Moathouse Dungeon
		ggv402 = game.global_vars[402]
		ggv403 = game.global_vars[403]
		if (ggv402 & (2**0) ) == 0:
			print "modifying moathouse... \n"
			modify_moathouse()
			
			ggv402 |= 2**0
			game.global_vars[402] = ggv402

		if moathouse_alerted() and (ggv403 & (2**0)) == 0:
			moathouse_reg()
			ggv403 |= 2**0
			game.global_vars[403] = ggv403
	elif (cur_map == 5008):
		print "Welcome Wench upstairs"
		for dude in game.party:
			if dude.type == obj_t_pc and dude.scripts[9] == 439 and get_f('pc_dropoff'):
				print "Attempting to remove " + str(dude)
				game.timevent_add(dude.obj_remove_from_all_groups,(dude), 150, 1)
		set_f('pc_dropoff', 0)	
	elif (cur_map == 5110):  ## Temple Ruined Building
		game.global_vars[491] |= 2**0
	elif (cur_map == 5111):  ## Temple Broken Tower - Exterior
		game.global_vars[491] |= 2**1
	elif (cur_map == 5065):  ## Temple Broken Tower - Interior
		game.global_vars[491] |= 2**2
	elif (cur_map == 5092):  ## Temple Escape Tunnel
		game.global_vars[491] |= 2**3
	elif (cur_map == 5112):  ## Temple Burnt Farmhouse
		game.global_vars[491] |= 2**4
	elif (cur_map == 5064):  ## Temple entrance level
		found_map_obj = 0
		for pc in game.party:
			if pc.item_find(11299):
				found_map_obj = 1
		if not found_map_obj:
			map_obj = game.obj_create(11299, game.leader.location)
			got_map_obj = 0
			pc_index = 0
			while got_map_obj == 0 and pc_index < len(game.party):
				if game.party[pc_index].is_unconscious() == 0 and game.party[pc_index].type == obj_t_pc:
					got_map_obj = game.party[pc_index].item_get(map_obj)
					if not got_map_obj:
						pc_index += 1
				else:
					pc_index += 1
			if got_map_obj:
				game.party[pc_index].scripts[9] = 435
				game.party[pc_index].begin_dialog( game.party[pc_index], 1200 )
			else:
				map_obj.object_flag_set(OF_OFF)
		if game.global_vars[455] & 2**7 != 0:
			for obj in game.obj_list_vicinity( lfa(500, 500), OLC_SCENERY | OLC_PORTAL ):
				if obj.obj_get_int( obj_f_secretdoor_flags) & 2**16: #OSDF_SECRET_DOOR
					obj.obj_set_int( obj_f_secretdoor_flags, obj.obj_get_int( obj_f_secretdoor_flags) | 2**17 )
	elif (cur_map == 5066):		## Temple Level 1 ##
		if get_v(455) & 1 == 0:
			record_time_stamp(460)
			set_v(455, get_v(455) | 1)
			modify_temple_level_1(attachee)
		if earth_alerted() and (get_v(454) & 1 == 0) and (game.global_vars[450] & 2**0 == 0)  and ( ( game.global_vars[450] & (2**13) ) == 0 ):
			set_v(454, get_v(454) | 1)
			earth_reg()
		xx, yy = location_to_axis(game.leader.location)
		if (xx - 421)**2 + (yy-589)**2 <= 400:
			game.global_vars[491] |= 2**5
		if (xx - 547)**2 + (yy-589)**2 <= 400:
			game.global_vars[491] |= 2**6
	elif (cur_map == 5067):		## Temple Level 2 ##
		if get_v(455) & 2 == 0:
			record_time_stamp(461)
			set_v(455, get_v(455) | 2)
			modify_temple_level_2(attachee)
		if water_alerted() and (   get_v(454) & 2 == 0  or  ( get_v(454)&(2**6+2**7)==2**6 )   ) and (game.global_vars[450] & 2**0 == 0) and ( ( game.global_vars[450] & (2**13) ) == 0 ):
			set_v(454, get_v(454) | 2)
			if get_v(454) & (2**6 + 2**7) == 2**6:
				set_v(454, get_v(454) | 2**7)	# indicate that Oohlgrist and co have been moved to Water
			water_reg()
		if air_alerted() and (get_v(454) & 4 == 0) and (game.global_vars[450] & 2**0 == 0) and ( ( game.global_vars[450] & (2**13) ) == 0 ):
			set_v(454 , get_v(454) | 4)
			air_reg()
		if fire_alerted() and (   get_v(454) & 2**3 == 0 or ( get_v(454)&(2**4+2**5)==2**4 )   ) and (game.global_vars[450] & 2**0 == 0) and ( ( game.global_vars[450] & (2**13) ) == 0 ):
			# Fire is on alert and haven't yet regrouped, or have already regrouped but Oohlgrist was recruited afterwards (2**5) and not transferred yet
			set_v(454, get_v(454) | 2**3)
			if get_v(454) & (2**4 + 2**5) == 2**4:
				set_v(454, get_v(454) | 2**5)	# indicate that Oohlgrist and co have been moved
			game.global_flags[154] = 1 # Make the Werewolf mirror shut up
			fire_reg()
		xx, yy = location_to_axis(game.leader.location)
		if (xx - 564)**2 + (yy-377)**2 <= 400:
			game.global_vars[491] |= 2**7
		elif (xx - 485)**2 + (yy-557)**2 <= 1600:
			game.global_vars[491] |= 2**8
		elif (xx - 485)**2 + (yy-503)**2 <= 400:
			game.global_vars[491] |= 2**8
	elif (cur_map == 5105):	## Temple Level 3 Lower (Thrommel, Scorpp, Falrinth etc.)
		if get_v(455) & 4 == 0:
			record_time_stamp(462)
			set_v(455, get_v(455) | 4)
		xx, yy = location_to_axis(game.leader.location)
		if (xx - 406)**2 + (yy-436)**2 <= 400: # Fire Temple Access (near groaning spirit)
			game.global_vars[491] |= 2**9
		elif (xx - 517)**2 + (yy-518)**2 <= 400: # Air Temple Access (troll keys)
			game.global_vars[491] |= 2**10
		elif (xx - 552)**2 + (yy-489)**2 <= 400: # Air Temple Secret Door (Scorpp Area)
			game.global_vars[491] |= 2**22
		elif (xx - 616)**2 + (yy-606)**2 <= 400: # Water Temple Access (lamia)
			game.global_vars[491] |= 2**11
		elif (xx - 639)**2 + (yy-450)**2 <= 1600: # Falrinth area
			game.global_vars[491] |= 2**12
			if game.global_vars[455] & 2**7 != 0: # Secret Door Reveal
				for obj in game.obj_list_vicinity( lfa(622,503), OLC_PORTAL | OLC_SCENERY ):
					if obj.obj_get_int( obj_f_secretdoor_flags) & 2**16: # OSDF_SECRET_DOOR
						obj.obj_set_int(obj_f_secretdoor_flags, obj.obj_get_int( obj_f_secretdoor_flags) | 2**17 )
		
			
	elif (cur_map == 5080): ## Temple Level 4
		if get_v(455) & 8 == 0:
			record_time_stamp(463)
			set_v(455, get_v(455) | 8)
		xx, yy = location_to_axis(game.leader.location)
		if (xx - 479)**2 + (yy-586)**2 <= 400:
			game.global_vars[491] |= 2**13
		elif (xx - 477)**2 + (yy-340)**2 <= 400:
			game.global_vars[491] |= 2**14
	elif (cur_map == 5106): ## secret spiral staircase
		game.global_vars[491] |= 2**15
	elif (cur_map == 5081): ## Air Node
		game.global_vars[491] |= 2**16
	elif (cur_map == 5082): ## Earth Node
		game.global_vars[491] |= 2**17
	elif (cur_map == 5083): ## Fire Node
		game.global_vars[491] |= 2**18
	elif (cur_map == 5084): ## Water Node
		game.global_vars[491] |= 2**19
	elif (cur_map == 5079): ## Zuggtmoy Level
		game.global_vars[491] |= 2**20
	return RUN_DEFAULT

def modify_temple_level_1(pc):
	# Gives Temple monsters and NPCs san_dying scripts, so the game recognizes the player slaughtering mobs
	#gnolls near southern entrance
	for gnollol in vlistxyr(558, 600, 14080, 25):
		gnollol.scripts[12] = 441
		#gnollol.destroy()
	for gnollol in vlistxyr(558, 600, 14079, 25):
		gnollol.scripts[12] = 441
		#gnollol.destroy()
	for gnollol in vlistxyr(558, 600, 14078, 25):
		gnollol.scripts[12] = 441
		#gnollol.destroy()

	# Rats
	for vaporrat in vlistxyr(497, 573, 14068, 30):
		vaporrat.scripts[12] = 441
		#vaporrat.destroy()
	for direrat in vlistxyr(440, 571, 14056, 15):
		direrat.scripts[12] = 441
		#direrat.destroy()
	for direrat in vlistxyr(534, 389, 14056, 15):
		direrat.scripts[12] = 441
		#direrat.destroy()


	#undead near secret staircase
	for skellygnoll in vlistxyr(462, 520, 14083, 100):
		skellygnoll.scripts[12] = 441
		#skellygnoll.destroy()
	for skellygnoll in vlistxyr(462, 520, 14082, 100):
		skellygnoll.scripts[12] = 441
		#skellygnoll.destroy()
	for skellygnoll in vlistxyr(462, 520, 14081, 100):
		skellygnoll.scripts[12] = 441
		#skellygnoll.destroy()
	for skellybone in vlistxyr(496, 515, 14107, 100):
		skellybone.scripts[12] = 441
		#skellybone.destroy()


	#Gnoll Leader area
	for gnoll_leader in vlistxyr(509, 534, 14066, 100):
		gnoll_leader.scripts[12] = 442
		#gnoll_leader.destroy()
	for gnoll in vlistxyr(518, 531, 14067, 66):
		gnoll.scripts[12] = 442
		#gnoll.destroy()
	for gnoll in vlistxyr(518, 531, 14078, 66):
		# Barbarian gnoll
		gnoll.scripts[12] = 442
		#gnoll.destroy()
	for gnoll in vlistxyr(518, 531, 14079, 66):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy() #replaces gnoll with non-DR version
		newgnoll = game.obj_create( 14631, gloc )
		newgnoll.rotation = grot		
		newgnoll.scripts[12] = 442
		#gnoll.destroy()
	for gnoll in vlistxyr(518, 531, 14080, 66):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy()
		newgnoll = game.obj_create( 14632, gloc )
		newgnoll.rotation = grot
		newgnoll.scripts[12] = 442
		#newgnoll.destroy()
	for gnoll in vlistxyr(511, 549, 14079, 33):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy()
		newgnoll = game.obj_create( 14631, gloc )
		newgnoll.rotation = grot
		newgnoll.scripts[12] = 442
		#newgnoll.destroy()
	for gnoll in vlistxyr(511, 549, 14080, 33):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy()
		newgnoll = game.obj_create( 14632, gloc )
		newgnoll.rotation = grot
		newgnoll.scripts[12] = 442
		#newgnoll.destroy()
	for ogre in vlistxyr(508, 536, 14249, 35):
		ogre.scripts[12] = 442
		#ogre.destroy()
	for bugbear in vlistxyr(508, 536, 14164, 35):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	#Earth critters near Ogre Chief
	for gnoll in vlistxyr(445, 538, 14078, 50):
		gnoll.scripts[12] = 442
		#gnoll.destroy()
	for gnoll in vlistxyr(445, 538, 14079, 50):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy()
		newgnoll = game.obj_create( 14631, gloc )
		newgnoll.rotation = grot
		newgnoll.scripts[12] = 442
		#newgnoll.destroy()
	for gnoll in vlistxyr(445, 538, 14080, 50):
		gloc = gnoll.location
		grot = gnoll.rotation
		gnoll.destroy()
		newgnoll = game.obj_create( 14632, gloc )
		newgnoll.rotation = grot
		newgnoll.scripts[12] = 442
		#newgnoll.destroy()
	for ogrechief in vlistxyr(467, 535, 14248, 50):
		ogrechief.scripts[12] = 444
		#ogrechief.destroy()
	for hobgoblin in vlistxyr(467, 535, 14188, 50):
		hobgoblin.scripts[12] = 442
		#hobgoblin.destroy()
	for goblin in vlistxyr(467, 535, 14184, 27):
		goblin.scripts[12] = 442
		#goblin.destroy()
	for goblin in vlistxyr(467, 535, 14186, 27):
		gloc = goblin.location
		grot = goblin.rotation
		goblin.destroy()
		newgob = game.obj_create( 14636, gloc )
		newgob.rotation = grot
		newgob.scripts[12] = 442
		#newgob.destroy()
	for bugbear in vlistxyr(467, 535, 14164, 27):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	#Temple Troops near Ogre Chief
	for troop in vlistxyr(440, 500, 14337, 30):
		troop.scripts[12] = 443
		#troop.destroy()
	for fighter in vlistxyr(440, 500, 14338, 30):
		fighter.scripts[12] = 443
		#fighter.destroy()


	#ghouls and ghasts near prisoners (Morgan etc.)
	for ghast in vlistxyr(545, 535, 14137, 50):
		ghast.scripts[12] = 441
		#ghast.destroy()
	for ghast in vlistxyr(550, 545, 14136, 50):
		ghast.scripts[12] = 441
		#ghast.destroy()
	for ghast in vlistxyr(545, 553, 14135, 50):
		ghast.scripts[12] = 441
		#ghast.destroy()
	for ghoul in vlistxyr(549, 554, 14095, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(549, 554, 14128, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(549, 554, 14129, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()


	#harpy area
	for harpy in vlistxyr(406, 564, 14243, 100):
		harpy.scripts[12] = 441
		#harpy.destroy()
	for harpy in vlistxyr(407, 545, 14243, 100):
		harpy.scripts[12] = 441
		#harpy.destroy()
	for ghast in vlistxyr(423, 541, 14135, 50):
		ghast.scripts[12] = 441
		#ghast.destroy()
	for ghast in vlistxyr(420, 547, 14136, 50):
		ghast.scripts[12] = 441
		#ghast.destroy()
	for ghoul in vlistxyr(413, 566, 14129, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(413, 566, 14128, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(413, 566, 14095, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(410, 526, 14129, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(410, 526, 14128, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()
	for ghoul in vlistxyr(410, 526, 14095, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()


	# Gray Ooze and Gelatinous Cube
	for gelatinouscube in vlistxyr(415, 599, 14139, 100):
		gelatinouscube.scripts[12] = 441
		#gelatinouscube.destroy()
	for grayooze in vlistxyr(415, 599, 14140, 100):
		grayooze.scripts[12] = 441
		#grayooze.destroy()


	#spiders near wonnilon hideout
	for spider in vlistxyr(438, 398, 14417, 50):
		spider.scripts[12] = 441
		#spider.destroy()

	#ghouls near wonnilon hideout
	for ghoul in vlistxyr(387, 398, 14128, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()

	#ghouls near northern entrance
	for ghoul in vlistxyr(459, 600, 14129, 100):
		ghoul.scripts[12] = 441
		#ghoul.destroy()

	#ogre near southern entrance
	for ogre in vlistxyr(511, 601, 14448, 100):
		ogre.scripts[12] = 441
		#ogre.destroy()


	#Temple Troop and bugbear doormen near Earth Commander
	for troop in vlistxyr(470, 483, 14337, 25):
		troop.scripts[12] = 443
		#troop.destroy()
	for bugbear in vlistxyr(470, 483, 14165, 25):
		bugbear.scripts[12] = 442
		#bugbear.destroy()

	
	#Temple Troops and bugbears near Earth Commander
	for earthcommander in vlistxyr(450, 470, 14156, 35):
		earthcommander.scripts[12] = 444
		#earthcommander.destroy()
	for lieutenant in vlistxyr(450, 470, 14339, 35):
		lieutenant.scripts[12] = 443
		#lieutenant.destroy()
	for troop in vlistxyr(450, 470, 14337, 35):
		troop.scripts[12] = 443
		#troop.destroy()
	for bugbear in vlistxyr(450, 470, 14165, 35):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	#Earth Altar
	for worshippers in vlistxyr(482, 392, 14337, 50):
		worshippers.scripts[12] = 443
		#worshippers.destroy()
	for earthelemental in vlistxyr(482, 392, 14381, 50):
		earthelemental.scripts[12] = 442
		#earthelemental.destroy()
	for largeearthelemental in vlistxyr(483, 420, 14296, 50):
		largeearthelemental.scripts[12] = 442
		#largeearthelemental.destroy()


	#Romag, Hartsch and their bugbear guards
	#for romag in vlistxyr(445, 445, 8045, 11):
	#	romag.scripts[12] = 444
	#	romag.destroy()
	for hartsch in vlistxyr(445, 445, 14154, 11):
		hartsch.scripts[12] = 444
		#hartsch.destroy()
	for bugbear in vlistxyr(445, 445, 14162, 11):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(445, 445, 14163, 11):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	# Bugbears north of Romag
	for bugbear in vlistxyr(427, 435, 14162, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(427, 435, 14164, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(427, 435, 14165, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(418, 443, 14163, 5):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(435, 427, 14163, 5):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(435, 427, 14164, 5):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	# Bugbear "Checkpoint"
	for bugbear in vlistxyr(504, 477, 14164, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(504, 477, 14162, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(504, 477, 14163, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(504, 477, 14165, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	# Bugbear "reservists"
	for bugbear in vlistxyr(524, 416, 14164, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(524, 416, 14163, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for bugbear in vlistxyr(524, 416, 14162, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()



	# Wonnilon area
	for zombie in vlistxyr(546, 418, 14092, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 418, 14123, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 418, 14127, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for bugbear in vlistxyr(546, 418, 14164, 35):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for zombie in vlistxyr(546, 435, 14092, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 435, 14124, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 435, 14125, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 435, 14126, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for zombie in vlistxyr(546, 435, 14127, 100):
		zombie.scripts[12] = 441
		#zombie.destroy()
	for bugbear in vlistxyr(546, 435, 14164, 35):
		bugbear.scripts[12] = 442
		#bugbear.destroy()


	# Turnkey
	for bugbear in vlistxyr(570, 460, 14165, 15):
		bugbear.scripts[12] = 442
		#bugbear.destroy()
	for turnkey in vlistxyr(570, 460, 14229, 15):
		turnkey.scripts[12] = 443
		#turnkey.destroy()


	# Ogre and Goblins
	for goblin in vlistxyr(563, 501, 14186, 50):
		goblin.scripts[12] = 441
		#goblin.destroy()
	for goblin in vlistxyr(563, 501, 14187, 50):
		goblin.scripts[12] = 441
		#goblin.destroy()
	for goblin in vlistxyr(563, 501, 14185, 50):
		goblin.scripts[12] = 441
		#goblin.destroy()
	for ogre in vlistxyr(563, 501, 14448, 50):
		ogre.scripts[12] = 441
		#ogre.destroy()


	# Stirges
	for stirge in vlistxyr(410, 491, 14182, 50):
		stirge.scripts[12] = 441
		#stirge.destroy()

	return

def modify_temple_level_2(pc):
	dummy = 1
	return


#104 - romag dead
#105 - belsornig dead
#106 - kelno dead
#107 - alrrem dead

def earth_alerted():
	if game.global_flags[104] == 1: ##romag is dead
		return 0
	if tpsts(512, 1*60*60) == 1:
	# an hour has passed since you defiled the Earth Altar
		return 1
	if tpsts(507, 1) == 1:
	# You've killed the Troop Commander
		return 1
	if tpsts(TS_CRITTER_THRESHOLD_CROSSED, 1):
		also_killed_earth_member = (tpsts(TS_EARTH_TROOP_KILLED_FIRST_TIME , 3*60) == 1) or (tpsts(TS_EARTH_CRITTER_KILLED_FIRST_TIME , 6*60) == 1)
		did_quest_1 = game.quests[43].state >= qs_completed
		if (not did_quest_1) or also_killed_earth_member:

			if tpsts(TS_CRITTER_THRESHOLD_CROSSED, 2*60*60): # two hours have passed since you passed critter deathcount threshold
				return 1
			if tpsts(TS_CRITTER_KILLED_FIRST_TIME, 48*60*60) == 1: #48 hours have passed since you first killed a critter and you've passed the threshold
				return 1
		# The second condition is for the case you've killed a critter, left to rest somewhere, and returned later, and at some point crossed the kill count threshold
	if (tpsts(510, 1) == 1 and tpsts(505, 24*60*60) == 1) or tpsts(510, 2*60*60):
	# Either two hours have passed since you passed Earth critter deathcount threshold, or 24 hours have passed since you first killed an Earth critter and you've passed the threshold
		return 1
	if (tpsts(511, 1) == 1 and tpsts(506, 12*60*60) == 1) or tpsts(511, 1*60*60):
	# Either 1 hour has passed since you passed troop deathcount threshold, or 12 hours have passed since you first killed a troop and you've passed the threshold
		return 1
	if tsc(457, 470) or tsc(458, 470) or tsc(459, 470): ##killed Belsornig, Kelno or Alrrem before completing 2nd earth quest
		return 1
	return 0

def water_alerted():
	if game.global_flags[105] == 1: 
	##belsornig is dead
		return 0
	if tsc(456,475) == 1 or tsc(458, 475) == 1 or tsc(459, 475) == 1: ##killed Romag, Kelno or Alrrem before accepting second water quest 
		return 1
	return 0

def air_alerted():
	if game.global_flags[106] == 1: 
		##kelno is dead
		return 0
	if tsc(456,483) or tsc(457, 483) or tsc(459, 483):
		##any of the other faction leaders are dead, and he hasn't yet given you that quest
		##Kelno doesn't take any chances
		return 1
	return 0

def fire_alerted():
	if game.global_flags[107] == 1: ##alrrem is dead
		return 0
	#if (game.global_flags[104] == 1 or game.global_flags[105] == 1 or game.global_flags[106] == 1):
		# For now - if one of the other Leaders is dead
		#return 1
	if tsc(456,517) or tsc(457, 517) or tsc(458, 517):
	# Have killed another High Priest without even having talked to him
	# Should suffice for him, since he's kind of crazy
		return 1
	return 0


################################################################
################################################################
################################################################
################################################################



def is_follower(name):
	for obj in game.party:
		if (obj.name == name):
			return 1
	return 0




def destroy_weapons(npc, item1, item2, item3):
	if (item1 != 0):
		moshe = npc.item_find(item1)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item2 != 0):
		moshe = npc.item_find(item2)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item3 != 0):
		moshe = npc.item_find(item3)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	return


def float_comment(attachee, line):
	attachee.float_line(line,game.leader)
	return
	
def daemon_float_comment(attachee, line):
	if attachee.type == obj_t_pc:
		attachee.scripts[9] = 439
		attachee.float_line(line,game.leader)
		attachee.scripts[9] = 0
	return

def proactivity(npc,line_no):
	npc.turn_towards(game.party[0])
	if (critter_is_unconscious(game.party[0]) != 1 and game.party[0].type == obj_t_pc and game.party[0].d20_query(Q_Prone) == 0 and npc.can_see(game.party[0])):
		game.party[0].begin_dialog(npc,line_no)
	else:
		for pc in game.party:
			npc.turn_towards(pc)
			if (critter_is_unconscious(pc) != 1 and pc.type == obj_t_pc and pc.d20_query(Q_Prone) == 0 and npc.can_see(pc)):
				pc.begin_dialog(npc,line_no)
	return


def tsc( var1, var2 ):
#time stamp compare
#check if event associated with var1 happened before var2
#if they happened in the same second, well... only so much I can do
	if (get_v(var1) == 0):
		return 0
	elif (get_v(var2) == 0):
		return 1
	elif (get_v(var1) < get_v(var2)):
		return 1
	else:
		return 0

def tpsts(time_var, time_elapsed):
# type: (object, long) -> long
# Has the time elapsed since [time stamp] greater than the specified amount?
	if get_v(time_var) == 0:
		return 0
	if game.time.time_game_in_seconds(game.time) > get_v(time_var) + time_elapsed:
		return 1
	return 0

def record_time_stamp(tvar, time_stamp_overwrite = 0):
	if get_v(str(tvar)) == 0 or time_stamp_overwrite == 1:
		set_v(str(tvar), game.time.time_game_in_seconds(game.time) )
	return


def pop_up_box(message_id):
	# generates popup box ala tutorial (without messing with the tutorial entries...)
	a = game.obj_create(11001, game.leader.location)
	a.obj_set_int(obj_f_written_text_start_line,message_id)
	game.written_ui_show(a)
	a.destroy()
	return



def paladin_fall():
	for pc in game.party:
		pc.condition_add('fallen_paladin')

def vlistxyr( xx, yy, name, radius ):
	greg = []
	for npc in game.obj_list_vicinity( lfa(xx,yy), OLC_NPC ):
		npc_x, npc_y = lta(npc.location)
		dist = sqrt((npc_x-xx)*(npc_x-xx) + (npc_y-yy)*(npc_y-yy))
		if (npc.name == name and dist <= radius):
			greg.append(npc)
	return greg


def can_see2(npc,pc):
	# Checks if there's an obstruction in the way (i.e. LOS regardless of facing)
	orot = npc.rotation ## Original rotation
	nx, ny = location_to_axis(npc.location)
	px, py = location_to_axis(pc.location)
	vx = px-nx
	vy = py-ny
	# (vx, vy) is a vector pointing from the PC to the NPC. 
	# Using its angle, we rotate the NPC and THEN check for sight.
	# After that, we return the NPC to its original facing.
	npc.rotation = 3.14159/2 - ( atan2(vy,vx) + 5*3.14159/4 )
	if npc.can_see(pc):
		npc.rotation = orot
		return 1
	npc.rotation = orot
	return 0

def can_see_party(npc):
	for pc in game.party[0].group_list():
		if can_see2(npc, pc) == 1:
			return 1
	return 0

def is_far_from_party(npc, dist = 20):
	# Returns 1 if npc is farther than specified distance from party
	for pc in game.party[0].group_list():
		if npc.distance_to(pc) < dist:
			return 0
	return 1



def is_safe_to_talk_rfv(npc, pc, radius = 20, facing_required = 0, visibility_required = 1):
	# visibility_required - Capability of seeing PC required (i.e. PC is not invisibile / sneaking)
	#	-> use can_see2(npc, pc)
	# facing_required - In addition, the NPC is actually looking at the PC's direction

	if visibility_required == 0:
		if ( pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	elif visibility_required == 1 and facing_required == 1:
		if ( npc.can_see(pc) == 1 and pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	elif visibility_required == 1 and facing_required != 1:
		if ( can_see2(npc, pc) == 1 and pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	return 0


def within_rect_by_corners(obj, ulx, uly, brx, bry):
	# refers to "visual" axes (edges parallel to your screen's edges rather than ToEE's native axes)
	xx, yy = location_to_axis(obj.location)
	if ( (xx - yy) <= (ulx-uly)) and ( (xx - yy) >= (brx-bry) ) and ( (xx + yy) >= (ulx + uly) ) and ( (xx+yy) <= (brx+bry) ):
		return 1
	return 0

	
def encroach(a,b):
	# A primitive way of making distant AI combatants who don't close the distances by themselves move towards the player
	b.turn_towards(a)
	if a.distance_to(b) < 30:
		return -1
	ax,ay = location_to_axis(a.location)
	bx,by = location_to_axis(b.location)
	dx = 0
	dy = 0
	if bx > ax:
		dx = 1
	elif bx < ax:
		dx = -1
	if by > ay:
		dy = 1
	elif by < ay:
		dy = -1
	if (ax-bx)**2 > (ay-by)**2: # if X distance is greater than Y distance, starting trying to encroach on the x axis
		aprobe = game.obj_create( 14631, location_from_axis(ax+dx, ay) ) # probe to see if I'm not going into a wall
		aprobe.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
		if can_see2(aprobe,a):
			aprobe.destroy()
			a.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			return 1
		else:
			aprobe.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			if can_see2(aprobe,a):
				aprobe.destroy()
				a.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				return 1
			else:
				aprobe.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				if can_see2(aprobe,a):
					aprobe.destroy()
					a.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
					return 1
				else:
					aprobe.destroy()
					return 0
	else:
		aprobe = game.obj_create( 14631, location_from_axis(ax+dx, ay) ) # probe to see if I'm not going into a wall
		aprobe.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
		if can_see2(aprobe,a):
			aprobe.destroy()
			a.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			return 1
		else:
			aprobe.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			if can_see2(aprobe,a):
				aprobe.destroy()
				a.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				return 1
			else:
				aprobe.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				if can_see2(aprobe,a):
					aprobe.destroy()
					a.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
					return 1
				else:
					aprobe.destroy()
					return 0
	return 0

def buffee( makom , det_range, buff_list, done_list ):
	# finds people that are on a 'to buff' list "buff_list" (name array), around location "makom", at range "det_range", that are not mentioned in "done_list"
	# e.g. in Alrrem's script you can find something like buffee( attachee.location, 15, [14344], [handle_to_other_werewolf] )
	xx0, yy0 = location_to_axis(makom)
	for darling in buff_list:
		for obj in game.obj_list_vicinity( makom, OLC_NPC ):
			xx1, yy1 = location_to_axis( obj.location )
			if obj.name == darling and obj.leader_get() == OBJ_HANDLE_NULL and not (obj in done_list) and ( (xx1-xx0)**2+ (yy1-yy0)**2 ) <= det_range**2:
				return obj
	return OBJ_HANDLE_NULL




def modify_moathouse():
	for obj in game.obj_list_vicinity(location_from_axis(490, 535), OLC_NPC):
		if obj.name in range(14074, 14078):
			obj.scripts[12] = 450
			obj.scripts[13] = 450
			obj.scripts[14] = 450
			obj.scripts[15] = 450
			obj.scripts[16] = 450
			if obj.name == 14077:
				obj.npc_flag_set(ONF_KOS)
				obj.scripts[22] = 450 #will kos
			obj.scripts[41] = 450
			obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
			obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
	for obj in game.obj_list_vicinity(location_from_axis(512, 549), OLC_NPC):
		if obj.name in range(14074, 14078):
			obj.scripts[12] = 450
			obj.scripts[13] = 450
			obj.scripts[14] = 450
			obj.scripts[15] = 450
			obj.scripts[16] = 450
			if obj.name == 14077:
				obj.npc_flag_set(ONF_KOS)
				obj.scripts[22] = 450 #will kos
			obj.scripts[41] = 450
			obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
			obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)


	return


def moathouse_alerted():
	if game.global_flags[363] == 1:
		# Bullied or attacked Sergeant at the door
		return 1
	else:
		ggv = game.global_vars
		bugbear_group_kill_ack = 0
		gnoll_group_kill_ack = 0
		lubash_kill_ack = 0
		ground_floor_brigands_kill_ack = 0
		if ggv[404] != 0 and ( game.time.time_game_in_seconds(game.time) > ggv[404] + 12*60*60):
			bugbear_group_kill_ack = 1
		if ggv[405] != 0 and ( game.time.time_game_in_seconds(game.time) > ggv[405] + 12*60*60):
			gnoll_group_kill_ack = 1
		if ggv[406] != 0 and ( game.time.time_game_in_seconds(game.time) > ggv[406] + 12*60*60):
			lubash_kill_ack = 1
		if ggv[407] != 0 and ( game.time.time_game_in_seconds(game.time) > ggv[407] + 48*60*60):
			ground_floor_brigands_kill_ack = 1

		return (  (ground_floor_brigands_kill_ack + lubash_kill_ack + gnoll_group_kill_ack + bugbear_group_kill_ack) >= 2  )

	return 0


def moathouse_reg():
	found_new_door_guy = 0
	for obj in game.obj_list_vicinity( location_from_axis(512, 549), OLC_NPC ):
		if obj.leader_get() != OBJ_HANDLE_NULL or obj.is_unconscious() == 1:
			continue
		xx, yy = location_to_axis(obj.location)
		if obj.name in [14074, 14075] and xx > 496 and yy > 544: 
			# Corridor guardsmen
			if xx == 497 and yy == 549: 
				# archer
				sps(obj, 639)
				obj.obj_set_int(obj_f_speed_walk, 1085353216)
				obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
				obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
				obj.move(location_from_axis(481, 530), 0,0)
				obj.rotation = 2.35
			elif xx == 507 and yy == 549: 
				# swordsman
				obj.destroy()

			elif xx == 515 and yy == 548: 
				# spearbearer
				sps(obj, 637)
				obj.obj_set_int(obj_f_speed_walk, 1085353216)
				obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
				obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
				obj.move(location_from_axis(483, 541), 0,0)
				obj.rotation = 4

			elif obj.name == 14075:
				# Door Sergeant - replace with a quiet sergeant
				obj.destroy()
				obj = game.obj_create( 14076, location_from_axis (476L, 541L) )
				obj.move(location_from_axis(476, 541), 0,0)
				obj.rotation = 4
				obj.scripts[12] = 450
				obj.scripts[13] = 450
				obj.scripts[14] = 450
				obj.scripts[15] = 450
				obj.scripts[16] = 450
				obj.scripts[41] = 450

	# Create a new door guy instead of the Sergeant
	if game.global_flags[37] == 0 and game.leader.reputation_has(15) == 0: # killed Lareth or cleared Moathouse
		obj = game.obj_create( 14074, location_from_axis (521L, 547L) )
		obj.move(location_from_axis(521, 547), 0,0)
		obj.rotation = 4
		obj.scripts[9] = 450
		obj.scripts[12] = 450
		obj.scripts[13] = 450
		obj.scripts[14] = 450
		obj.scripts[15] = 450
		obj.scripts[16] = 450
		obj.scripts[19] = 450
		obj.scripts[41] = 450

	return








def lnk(loc_0 = -1, xx = -1, yy = -1, name_id = -1, stun_name_id = -1):
	# Locate n' Kill!

	if type(stun_name_id) == type(-1):
		stun_name_id = [stun_name_id]
	if type(name_id) == type(-1):
		name_id = [name_id]

	if loc_0 == -1 and xx == -1 and yy == -1:
		loc_0 = game.leader.location
	elif xx != -1 and yy != -1:
		loc_0 = location_from_axis(xx, yy) # Needs location_from_axis from utilities.py
	else:
		loc_0 = game.leader.location

	if name_id == [-1]:
		for obj in game.obj_list_vicinity(loc_0, OLC_NPC):
			if (    obj.reaction_get(game.party[0]) <= 0 or obj.is_friendly(game.party[0]) == 0     )   and     ( obj.leader_get() == OBJ_HANDLE_NULL and obj.object_flags_get() & OF_DONTDRAW == 0):
				if not obj.name in stun_name_id:
					damage_dice = dice_new( '50d50' )
					obj.damage( game.party[0], 0, damage_dice )
					obj.damage( game.party[0], D20DT_FIRE, damage_dice )
					obj.damage( game.party[0], D20DT_COLD, damage_dice )
					obj.damage( game.party[0], D20DT_MAGIC, damage_dice )
				else:
					damage_dice = dice_new( '50d50' )
					obj.damage( OBJ_HANDLE_NULL, D20DT_SUBDUAL, damage_dice )
	else:
		for obj in game.obj_list_vicinity(loc_0, OLC_NPC):
			if obj.name in (name_id+stun_name_id) and ( obj.reaction_get(game.party[0]) <= 0 or obj.is_friendly(game.party[0]) == 0) and (obj.leader_get() == OBJ_HANDLE_NULL and obj.object_flags_get() & OF_DONTDRAW == 0):
				if not (obj.name in stun_name_id):
					damage_dice = dice_new( '50d50' )
					obj.damage( game.party[0], D20DT_BLUDGEONING, damage_dice )
					obj.damage( game.party[0], D20DT_FIRE, damage_dice )
					obj.damage( game.party[0], D20DT_COLD, damage_dice )
					obj.damage( game.party[0], D20DT_MAGIC, damage_dice )
				else:
					damage_dice = dice_new( '50d50' )
					if is_unconscious(obj) == 0:
						obj.damage( OBJ_HANDLE_NULL, D20DT_SUBDUAL, damage_dice )
					for pc in game.party:
						obj.ai_shitlist_remove( pc )


	return

def loot_items( loot_source = OBJ_HANDLE_NULL, pc=-1 , loot_source_name = -1, xx = -1, yy = -1, item_proto_list = [], loot_money_and_jewels_also = 1, autoloot = 1, autoconvert_jewels = 1, item_autoconvert_list = []):
	if get_f('qs_autoloot') != 1:
		return
	if get_f('qs_autoconvert_jewels') != 1:
		autoconvert_jewels = 0
	money_protos = range(7000, 7004) # Note that the range actually extends from 7000 to 7003
	gem_protos = [12010] + range(12034, 12045)
	jewel_protos = range(6180, 6198)
	potion_protos = [8006, 8007]

	tank_armor_0 = []
	barbarian_armor_0 = []
	druid_armor_0 = []
	wizard_items_0 = []

	autosell_list = []
	autosell_list += range(4002, 4106 )
	autosell_list += range(4113, 4120)
	autosell_list += range(4155, 4191)
	autosell_list += range(6001, 6048)
	autosell_list += [6055, 6056] + [6059, 6060]  + range(6062, 6073)
	autosell_list += range(6074, 6082)
	autosell_list += [6093, 6096, 6103, 6120, 6123, 6124]
	autosell_list += range(6142, 6153)
	autosell_list += range(6153, 6159)
	autosell_list += range(6163, 6180)
	autosell_list += range(6202, 6239 )

	autosell_exclude_list = []
	autosell_exclude_list += [4016, 4017, 4025, 4028] # Frag, Scath, Excal, Flam Swo +1
	autosell_exclude_list += [4047, 4057, 4058] # Scimitar +1, Dagger +2, Dager +1
	autosell_exclude_list += [4078, 4079] # Warha +1, +2
	autosell_exclude_list += range(4081, 4087) # Longsword +1 ... +5, Unholy Orc ax+1
	autosell_exclude_list += [4098] # Battleaxe +1
	autosell_exclude_list += [4161] # Shortsword +2
	autosell_exclude_list += [5802] # Figurine name IDs - as per protos.tab
	autosell_exclude_list += [6015, 6017, 6031, 6039, 6058, 6073, 6214, 6215, 6219]
	autosell_exclude_list += [6239, 12602]
	autosell_exclude_list += [8006, 8007, 8008, 8101] # Potions of Cure mod, serious & Haste
	# 6015 - eye of flame cloak
	# 6017 - gnome ring
	# 6031 - eyeglasses
	# 6039 - Full Plate
	# 6048 - Prince Thrommel's Plate
	# 6058 - Cloak of Elvenkind
	# 6073 - Wooden Elvish Shield
	# 6214, 6215 - Green & Purple (resp.) Elven chain
	# 6219 - Senshock robes
	# 6239 - Darley's Necklace
	# 12602 - Hill Giant's Head
	for qqq in autosell_exclude_list:
		if qqq in autosell_list:
			autosell_list.remove(qqq)

	if loot_money_and_jewels_also == 1:
		if type(item_proto_list) == type([]):
			item_proto_list = item_proto_list + money_protos + gem_protos + jewel_protos + potion_protos
		else:
			item_proto_list = [item_proto_list] + money_protos + gem_protos + jewel_protos + potion_protos
	elif type(item_proto_list) == type(1):
		item_proto_list = [item_proto_list]

	# pc - Who will take the loot?
	if pc == -1:
		pc = game.leader
	# loc_0 - Where will the loot be sought?
	if xx == -1 or yy == -1:
		loc_0 = pc.location
	else:
		loc_0 = location_from_axis(xx, yy)

	if loot_source != OBJ_HANDLE_NULL:
		for pp in (item_proto_list + item_autoconvert_list):
			if type(pp) == type(1):
				if pp in item_autoconvert_list:
					pp_1 = loot_source.item_find_by_proto(pp)
					if pp_1 != OBJ_HANDLE_NULL:
						if pp_1.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
							autosell(pp_1)
				elif pc.item_get( loot_source.item_find_by_proto(pp) ) == 0:
					for obj in game.party:
						if obj.item_get( loot_source.item_find_by_proto(pp) ) == 1:
							break
	else:
		if loot_source_name != -1:
			if type(loot_source_name) == type(1):
				loot_source_name = [loot_source_name]
		else:
			loot_source_name = [-1]
		for robee in game.obj_list_vicinity(loc_0, OLC_NPC | OLC_CONTAINER | OLC_ARMOR | OLC_WEAPON | OLC_GENERIC):
			if not robee in game.party[0].group_list() and (robee.name in loot_source_name or loot_source_name == [-1]):
				if (robee.type == obj_t_weapon) or (robee.type == obj_t_armor) or (robee.type == obj_t_generic):
					if robee.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
						if robee.name in autosell_list + item_autoconvert_list:
							autosell_item(robee)
						elif robee.name in autosell_exclude_list:
							if pc.item_get(robee) == 0:
								for obj in game.party:
									if obj.item_get(robee) == 1:
										break
				if robee.type == obj_t_npc:
					for qq in range(0, 16):
						qq_item_worn = robee.item_worn_at(qq)
						if qq_item_worn != OBJ_HANDLE_NULL and qq_item_worn.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
							if qq_item_worn.name in (autosell_list + item_autoconvert_list):
								autosell_item(qq_item_worn)
				for item_proto in (item_proto_list + item_autoconvert_list):
					item_sought = robee.item_find_by_proto(item_proto)
					if  item_sought != OBJ_HANDLE_NULL and item_sought.item_flags_get() & OIF_NO_DISPLAY == 0:
						if (  (item_proto in ( gem_protos + jewel_protos ) ) and autoconvert_jewels == 1) or (item_proto in item_autoconvert_list):
							autosell_item(item_sought, item_proto, pc)
						elif pc.item_get(item_sought) == 0:
							for obj in game.party:
								if obj.item_get(item_sought) == 1:
									break
	return

def sell_modifier():
	highest_appraise = -999
	for obj in game.party:
		if obj.skill_level_get(skill_appraise) > highest_appraise:
			highest_appraise = obj.skill_level_get(skill_appraise)
	for pc in game.party:
		if pc.stat_level_get(stat_level_wizard) > 1:
			highest_appraise = highest_appraise + 2 # Heroism / Fox's Cunning bonus
			break
	for pc in game.party:
		if pc.stat_level_get(stat_level_bard) > 1:
			highest_appraise = highest_appraise + 2 # Inspire Competence bonus
			break
	if highest_appraise > 19:
		return 0.97
	elif highest_appraise < -13:
		return 0
	else:
		return 0.4 + float(highest_appraise)*0.03

def appraise_tool( obj ):
	# Returns what you'd get for selling it
	aa = sell_modifier()
	return int( aa * obj.obj_get_int(obj_f_item_worth) )

def s_roundoff( app_sum ):
	if app_sum <= 1000:
		return app_sum
	if app_sum > 1000 and app_sum <= 10000:
		return 10 * int( (int(app_sum) / 10 ) )
	if app_sum > 10000 and app_sum <= 100000:
		return 100 * int( (int(app_sum) / 100 ) )
	if app_sum > 100000 and app_sum <= 1000000:
		return 1000 * int( (int(app_sum) / 1000 ) )

def autosell_item(item_sought = OBJ_HANDLE_NULL, item_proto = -1, pc = -1, item_quantity = 1, display_float = 1):
	
	if item_sought == OBJ_HANDLE_NULL:
		return
	if pc == -1:
		pc = game.leader
	if item_proto == -1:
		item_proto = item_sought.name

	autoconvert_copper =  appraise_tool(item_sought) * item_sought.obj_get_int(obj_f_item_quantity)
	pc.money_adj( autoconvert_copper )
	item_sought.object_flag_set(OF_OFF)
	item_sought.item_flag_set( OIF_NO_DISPLAY )
	item_sought.item_flag_set( OIF_NO_LOOT )

	if display_float == 1 and autoconvert_copper > 5000 or display_float == 2:
		pc.float_mesfile_line( 'mes\\script_activated.mes', 10000, 2 )
		pc.float_mesfile_line( 'mes\\description.mes', item_proto, 2 )
		pc.float_mesfile_line( 'mes\\transaction_sum.mes', ( s_roundoff(autoconvert_copper/100) ), 2 )

	return

def giv(pc, proto_id, in_group = 0):	
	if in_group == 0:
		if pc.item_find_by_proto(proto_id) == OBJ_HANDLE_NULL:
			create_item_in_inventory( proto_id, pc )
	else:
		foundit = 0
		for obj in game.party:
			if obj.item_find_by_proto(proto_id) != OBJ_HANDLE_NULL:
				foundit = 1
		if foundit == 0:
			create_item_in_inventory( proto_id, pc )
			return 1
		else:
			return 0
	return


def cnk(proto_id, do_not_destroy = 0, how_many = 1, timer = 0):
	# Create n' Kill
	# Meant to simulate actually killing the critter
	#if timer == 0:
	for pp in range(0, how_many):
		a = game.obj_create(proto_id, game.leader.location)
		damage_dice = dice_new( '50d50' )
		a.damage( game.party[0], 0, damage_dice )
		if do_not_destroy != 1:
			a.destroy()
	#else:
	#	for pp in range(0, how_many):
	#		game.timevent_add( cnk, (proto_id, do_not_destroy, 1, 0), (pp+1)*20 )
	return




################
################
### AUTOKILL ###
################
################







def autokill(cur_map, autoloot = 1, is_timed_autokill = 0):

	#if (cur_map in range(5069, 5078) ): #random encounter maps
	#	## Skole Goons
	#	flash_signal(0)
	#	if get_f('qs_autokill_nulb'):
	#		if get_v('qs_skole_goon_time') == 0:
	#			set_v('qs_skole_goon_time', 500)
	#			game.timevent_add( autokill, (cur_map), 100 )
	#			flash_signal(1)
	#		if get_v('qs_skole_goon_time') == 500:
	#			flash_signal(2)
	#			lnk(name_id = [14315])
	#			#14315 - Skole Goons
	#			loot_items(loot_source_name = [14315]) # Skole goons
		#if get_f('qs_is_repeatable_encounter'):
		#	lnk()
		#	loot_items()

################
###  HOMMLET   #
################

	if (cur_map == 5001): # Hommlet Exterior
		if get_v('qs_emridy_time') == 1500:
			game.quests[100].state = qs_completed
			bro_smith = OBJ_HANDLE_NULL
			for obj in game.obj_list_vicinity(location_from_axis(571, 434), OLC_NPC):
				if obj.name == 20005:
					bro_smith = obj
			if bro_smith != OBJ_HANDLE_NULL:
				party_transfer_to(bro_smith, 12602)
				game.global_flags[979] = 1
			set_v('qs_emridy_time', 2000)


		if get_f('qs_arena_of_heroes_enable'):
			if get_f('qs_lareth_dead'):
				game.global_vars[974] = 2 # Simulate having talked about chest
				game.global_vars[705] = 2 # Simulate having handled chest
				if get_f('qs_book_of_heroes_given') == 0:
					giv(game.leader, 11050, 1) # Book of Heroes
					giv(game.leader, 12589, 1) # Horn of Fog
					set_f('qs_book_of_heroes_given')
				game.global_vars[702] = 1 # Make sure Kent doesn't pester
			if game.global_vars[994] == 0:
				game.global_vars[994] = 1 # Skip Master of the Arena chatter



	if (cur_map == 5008): # Welcome Wench Upstairs
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Barbarian Elf
				lnk(xx=482, yy=476, name_id = 8717)
				loot_items(loot_source_name = 8717, item_autoconvert_list = [6396, 6045, 6046, 4204])
				game.global_vars[961] = 4


##################
###  MOATHOUSE   #
##################

	if (cur_map == 5002): # Moathouse Exterior
		if get_f('qs_autokill_moathouse') == 1:
			lnk(xx=469, yy=524, name_id = 14057) # giant frogs
			lnk(xx=492, yy=523, name_id = 14057) # giant frogs
			lnk(xx=475, yy=505, name_id = 14057) # giant frogs
			loot_items(xx=475, yy=505, item_proto_list = [6270], loot_source_name = 14057, autoloot = autoloot) # Jay's Ring

			lnk(xx=475, yy=460, name_id = 14070) # courtyard brigands
			loot_items(xx=475, yy=460, autoloot = autoloot)

			if get_v('qs_moathouse_ambush_time') == 0 and get_f('qs_lareth_dead') == 1:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_ambush_time', 500)
			elif get_v('qs_moathouse_ambush_time') == 500:
				lnk(xx = 478, yy = 460, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				lnk(xx = 430, yy = 444, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				loot_items(xx=478, yy=460)
				loot_items(xx=430, yy=444)
				set_v('qs_moathouse_ambush_time', 1000)

		if get_f('qs_autokill_temple') == 1:
			lnk(xx=503, yy=506, name_id = [14507, 14522] ) # Boars
			lnk(xx=429, yy=437, name_id = [14052, 14053] ) # Bears
			lnk(xx=478, yy=448, name_id = [14600, 14674, 14615, 14603, 14602, 14601] ) # Undead
			lnk(xx=468, yy=470, name_id = [14674, 14615, 14603, 14602, 14601] ) # Undead





	if (cur_map == 5003): # Moathouse Tower
		if get_f('qs_autokill_moathouse') == 1:
			lnk(name_id = 14047) # giant spider


	if (cur_map == 5004): # Moathouse Upper floor
		if get_f('qs_autokill_moathouse') == 1:
			lnk(xx = 476, yy = 493, name_id = 14088) # Huge Viper
			lnk(xx = 476, yy = 493, name_id = 14182) # Stirges

			lnk(xx = 473, yy = 472, name_id = [14070, 14074, 14069]) # Backroom brigands
			loot_items(xx=473, yy=472, autoloot = autoloot)
			lnk(xx = 502, yy = 476, name_id = [14089, 14090]) # Giant Tick & Lizard
			loot_items(xx=502, yy=472, autoloot = autoloot, item_proto_list = [6050])

		if get_f('qs_autokill_temple') == 1 and game.global_vars[972] == 2:
			if get_v('qs_moathouse_respawn__upper_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_respawn__upper_time', 500)
			if get_v('qs_moathouse_respawn__upper_time') == 500:
				lnk(xx=476, yy=493, name_id = [14138, 14344, 14391] ) # Lycanthropes
				lnk(xx = 502, yy = 476, name_id = [14295, 14142]) # Basilisk & Ochre Jelly


	if (cur_map == 5005): # Moathouse Dungeon
		if get_f('qs_autokill_moathouse') == 1:

			lnk(xx = 416, yy = 439, name_id = 14065) # Lubash
			loot_items(xx=416, yy=439, item_proto_list = [6058], loot_source_name = 14065 , autoloot = autoloot)
			game.global_flags[55] = 1 # Freed Gnomes
			game.global_flags[991] = 1 # Flag For Verbobonc Gnomes

			lnk(xx = 429, yy = 413, name_id = [14123, 14124, 14092, 14126, 14091]) # Zombies, Green Slime
			lnk(xx = 448, yy = 417, name_id = [14123, 14124, 14092, 14126]) # Zombies
			loot_items(xx=448, yy=417, item_proto_list = 12105, loot_source_name = -1 , autoloot = autoloot)



			lnk(xx = 450, yy = 519, name_id = range(14170, 14174) + range(14213, 14217) ) # Bugbears
			lnk(xx = 430, yy = 524, name_id = range(14170, 14174) + range(14213, 14217) ) # Bugbears
			loot_items(xx=450, yy=519 , autoloot = autoloot)
			loot_items(xx=430, yy=524 , autoloot = autoloot)

			if len(game.party) < 4 and get_v('AK5005_Stage') < 1:
				set_v('AK5005_Stage', get_v('AK5005_Stage') + 1)
				return
				
			# Gnolls and below
			lnk(xx = 484, yy = 497, name_id = [14066, 14067, 14078, 14079, 14080]) # Gnolls
			lnk(xx = 484, yy = 473, name_id = [14066, 14067, 14078, 14079, 14080]) # Gnolls
			loot_items(xx=484, yy=497 , autoloot = autoloot)
			loot_items(xx=484, yy=473 , autoloot = autoloot)

			lnk(xx = 543, yy = 502, name_id = 14094) # Giant Crayfish

			lnk(xx = 510, yy = 447, name_id = [14128, 14129, 14095]) # Ghouls

			if len(game.party) < 4 and get_v('AK5005_Stage') < 2   or  (  len(game.party) < 8 and get_v('AK5005_Stage') < 1    ):
				set_v('AK5005_Stage', get_v('AK5005_Stage') + 1)
				return

			lnk(xx = 515, yy = 547, name_id = [14074, 14075]) # Front Guardsmen
			loot_items(xx=515, yy=547 , autoloot = autoloot)

			lnk(xx = 485, yy = 536, name_id = [14074, 14075, 14076, 14077]) # Back Guardsmen
			loot_items(xx=485, yy=536 , loot_source_name = [14074, 14075, 14076, 14077], autoloot = autoloot) # Back guardsmen

			from py00060lareth import create_spiders
			if get_f('qs_lareth_spiders_spawned') == 0:
				create_spiders(game.leader, game.leader)
				set_f('qs_lareth_spiders_spawned', 1)
			lnk(xx = 480, yy = 540, name_id = [8002, 14397, 14398, 14620]) # Lareth & Spiders
			set_f('qs_lareth_dead')
			lnk(xx = 530, yy = 550, name_id = [14417]) # More Spiders
			loot_items(xx=480, yy=540 , item_proto_list = ([4120, 6097, 6098, 6099, 6100, 11003] + range(9001, 9688) ) , loot_source_name = [8002, 1045], autoloot = autoloot) # Lareth & Lareth's Dresser
			loot_items(xx=480, yy=540, item_autoconvert_list = [4194])
### RESPAWN
		if get_f('qs_autokill_temple') == 1 and game.global_vars[972] == 2:
			if get_v('qs_moathouse_respawn_dungeon_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_respawn_dungeon_time', 500)

			if get_v('qs_moathouse_respawn__upper_time') == 500:
				lnk(xx = 416, yy = 439, name_id = 14141) # Crystal Oozes

				# Bodaks, Shadows and Groaning Spirit
				lnk(xx = 436, yy = 521, name_id = [14328, 14289, 14280]) 

				# Skeleton Gnolls
				lnk(xx = 486, yy = 480, name_id = [14616, 14081, 14082, 14083]) 
				lnk(xx = 486, yy = 495, name_id = [14616, 14081, 14082, 14083]) # Skeleton Gnolls

				# Witch
				lnk(xx = 486, yy = 540, name_id = [14603, 14674, 14601, 14130, 14137, 14328, 14125, 14110, 14680]) 
				loot_items(xx = 486, yy = 540, item_proto_list = [11098, 6273, 4057,6263, 4498], item_autoconvert_list = [4226, 6333, 5099])


	if (cur_map == 5091): # Cave Exit
		if get_f('qs_autokill_moathouse') == 1:
			if get_v('qs_moathouse_ambush_time') == 0 and get_f('qs_lareth_dead') == 1:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_ambush_time', 500)
			elif get_v('qs_moathouse_ambush_time') == 500:
				lnk(xx = 500, yy = 490, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				lnk(xx = 470, yy = 485, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				loot_items(xx=500, yy=490)
				loot_items(xx=470, yy=490)
				set_v('qs_moathouse_ambush_time', 1000)




	if (cur_map == 5094): # Emridy Meadows
		if get_f('qs_autokill_moathouse') == 1:
			if get_v('qs_emridy_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_emridy_time', 500)
			elif get_v('qs_emridy_time') == 500:
				set_v('qs_emridy_time', 1000)
				game.timevent_add( autokill, (cur_map), 500 )

				lnk(xx = 467, yy = 383, name_id = [14603, 14600]) # NW Skeletons
				loot_items(xx=467, yy=380)

				lnk(xx = 507, yy = 443, name_id = [14603, 14600]) # W Skeletons
				lnk(xx = 515, yy = 421, name_id = [14603, 14600]) # W Skeletons
				loot_items(xx=507, yy=443)
				loot_items(xx=515, yy=421)

				lnk(xx = 484, yy = 487, name_id = [14603, 14600, 14616, 14615]) # Rainbow Rock 1
				lnk(xx = 471, yy = 500, name_id = [14603, 14600, 14616, 14615]) # Rainbow Rock 1
				loot_items(xx=484, yy=487)

				loot_items(xx=484, yy=487, loot_source_name = [1031], item_proto_list = [12024])



				if get_f('qs_rainbow_spawned') == 0:
					set_f('qs_rainbow_spawned', 1)
					#py00265rainbow_rock.san_use(game.leader, game.leader)
					#san_use(game.leader, game.leader)
					#game.particles( "sp-summon monster I", game.leader)
					for qq in game.obj_list_vicinity( location_from_axis(484, 487), OLC_CONTAINER ):
						if qq.name == 1031:
							qq.object_script_execute( qq, 1 )
				lnk(xx = 484, yy = 487, name_id = [14602, 14601]) # Rainbow Rock 2
				loot_items(xx=484, yy=487)


				#game.timevent_add( autokill, (cur_map), 1500 )

				lnk(xx = 532, yy = 540, name_id = [14603, 14600]) # SE Skeletons
				loot_items(xx=540, yy=540)



				lnk(xx = 582, yy = 514, name_id = [14221, 14053]) # Hill Giant
			elif get_v('qs_emridy_time') == 1000:
				set_v('qs_emridy_time', 1500)
				loot_items(xx=582, yy=514)
				loot_items(xx=582, yy=514, item_proto_list = [12602])
				if game.leader.item_find_by_proto(12602) == OBJ_HANDLE_NULL:
					create_item_in_inventory(12602, game.leader)

##################
###  NULB	 #
##################


	if (cur_map == 5051): # Nulb Outdoors

		if get_f('qs_autokill_temple') == 1:
			game.global_vars[972] = 2 # Simulate Convo with Kent

		if get_f('qs_autokill_nulb') == 1:
			# Spawn assassin
			game.global_flags[277] = 1 # Have met assassin
			game.global_flags[292] = 1
			if get_f('qs_assassin_spawned') == 0:
				a = game.obj_create(14303, game.leader.location)
				lnk(name_id = 14303)
				loot_items(loot_source_name = 14303, item_proto_list = [6315, 6199, 4701, 4500, 8007, 11002], item_autoconvert_list = [6046])
				set_f('qs_assassin_spawned')

			game.global_flags[356] = 1 # Met Mickey
			game.global_flags[357] = 1 # Mickey confessed to taking Orb
			game.global_flags[321] = 1 # Met Mona
			record_time_stamp('s_skole_goons')

			game.quests[41].state = qs_completed # Preston's Tooth Ache
			game.global_flags[94] = 1 # Nulb House is yours	
			game.global_flags[315] = 1 # Purchased Serena's Freedom	
			game.quests[60].state = qs_completed # Mona's Orb
			game.quests[63].state = qs_completed # Bribery for justice


			if get_f('qs_killed_gar') == 1:
				game.quests[35].state = qs_completed # Grud's story
				game.leader.reputation_add( 25 )


	if (cur_map == 5068): # Imeryd's Run
		if get_f('qs_autokill_nulb') == 1:
			lnk(xx = 485, yy = 455, name_id = ([14279] + range(14084, 14088))  ) # Hag & Lizards
			#lnk(xx = 468, yy = 467, name_id = ([14279] + range(14084, 14088))  ) # Hag & Lizards
			loot_items(xx=485, yy = 455)


			lnk(xx = 460, yy = 480, name_id = [14329]) # Gar
			loot_items(xx=460, yy=480, item_proto_list = [12005]) # Gar Corpse + Lamia Figurine
			loot_items(xx=460, yy=500, item_proto_list = [12005]) # Lamia Figurine - bulletproof
			set_f('qs_killed_gar')


			lnk(name_id = [14445, 14057]) # Kingfrog, Giant Frog
			loot_items(xx=476, yy = 497, item_proto_list = [4082, 6199, 6082, 4191, 6215, 5006])

	if (cur_map == 5052): # Boatmen's Tavern
		if get_f('qs_autokill_nulb') == 1:
			if game.global_flags[281] == 1: # Have had Skole Goons Encounter
				lnk(name_id = [14315, 14134]) # Skole + Goon
				loot_items(loot_source_name = [14315, 14134], item_proto_list = [6051, 4121])
				for obj_1 in game.obj_list_vicinity(game.leader.location, OLC_NPC):
					for pc_1 in game.party[0].group_list():
						obj_1.ai_shitlist_remove( pc_1 )
						obj_1.reaction_set( pc_1, 50 )

	if (cur_map == 5057): # Snakepit Brothel
		if get_f('qs_autokill_nulb') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				lnk(xx = 508, yy= 485, name_id = 8718)
				loot_items(xx = 508, yy = 485, loot_source_name = 8718, item_autoconvert_list = [4443, 6040, 6229])
				game.global_vars[961] = 6


	if (cur_map == 5060): # Waterside Hostel
		if get_f('qs_autokill_nulb') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
			# Thieving Dala
				game.quests[37].state = qs_completed
				lnk(xx = 480, yy= 501, name_id = [14147, 14146, 14145, 8018, 14074], stun_name_id = [14372, 14373])
				loot_items(xx=480, yy= 501, loot_source_name = [14147, 14146, 14145, 8018, 14074])
				for obj_1 in game.obj_list_vicinity(location_from_axis(480, 501), OLC_NPC):
					for pc_1 in game.party[0].group_list():
						obj_1.ai_shitlist_remove( pc_1 )
						obj_1.reaction_set( pc_1, 50 )

				

##########################
###  HICKORY BRANCH	 #
##########################

	if (cur_map == 5095): # Hickory Branch Exterior
		if get_f('qs_autokill_nulb'):
			# First party, near Noblig
			lnk(xx = 433, yy = 538, name_id = [14467, 14469, 14470, 14468, 14185]) 
			loot_items(xx=433, yy = 538, item_autoconvert_list = [4201, 4209, 4116, 6321]) # Shortbow, Spiked Chain, Short Spear, Marauder Armor

			# NW of Noblig
			lnk(xx = 421, yy = 492, name_id = [14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=421, yy = 492, item_autoconvert_list = [4201, 4209, 4116])

			# Wolf Trainer Group
			lnk(xx = 366, yy = 472, name_id = [14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=366, yy = 472, item_autoconvert_list = [4201, 4209, 4116])

			# Ogre Shaman Group
			lnk(xx = 449, yy = 455, name_id = [14249, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=449, yy = 455, item_autoconvert_list = [4201, 4209, 4116])

			# Orc Shaman Group
			lnk(xx = 494, yy = 436, name_id = [14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=494, yy = 436, item_autoconvert_list = [4201, 4209, 4116])

			# Cave Entrance Group
			lnk(xx = 527, yy = 380, name_id = [14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=527, yy = 380, item_autoconvert_list = [4201, 4209, 4116])
			
			# Dire Bear
			lnk(xx = 548, yy = 430, name_id = [14506])

			# Cliff archers
			lnk(xx = 502, yy = 479, name_id = [14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=502, yy = 479, item_autoconvert_list = [4201, 4209, 4116])
			
			# Giant Snakes
			lnk(xx = 547, yy = 500, name_id = [14449])
			loot_items(xx=547, yy = 500, item_autoconvert_list = [4201, 4209, 4116])

			# Owlbear
			lnk(xx = 607, yy = 463, name_id = [14046])


			# Dokolb area
			lnk(xx = 450, yy = 519, name_id = [14640, 14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=450, yy = 519, item_autoconvert_list = [4201, 4209, 4116])


			# South of Dokolb Area
			lnk(xx = 469, yy = 548, name_id = [14188, 14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=469, yy = 548, item_autoconvert_list = [4201, 4209, 4116])


	if (cur_map == 5115): # Hickory Branch Cave
		if get_f('qs_autokill_nulb'):
			if get_v('qs_hickory_cave_timer') == 0:
				set_v('qs_hickory_cave_timer', 500) 
				game.timevent_add(autokill, (cur_map), 500)
			if get_v('qs_hickory_cave_timer') == 500:
				lnk()
				loot_items(item_proto_list = [4086, 6106, 10023], item_autoconvert_list = [6143, 4110, 4241, 4242, 4243, 6066, 4201, 4209, 4116])
				loot_items(xx = 490, yy = 453, item_proto_list = [4078, 6252, 6339, 6091], item_autoconvert_list = [6304, 4240, 6161, 6160, 4087, 4204])


	if (cur_map == 5191): # Minotaur Lair
		if get_f('qs_autokill_nulb'):
			lnk(xx = 492, yy = 486)
			loot_items(492, 490, item_proto_list = [4238, 6486, 6487])


##########################
###  ARENA OF HEROES	 #
##########################

	if (cur_map == 5119): # AoH
		if get_f('qs_autokill_temple'):
			#game.global_vars[994] = 3
			dummy = 1


##########################
###  MOATHOUSE RESPAWN	 #
##########################

	if (cur_map == 5120): # Forest Drow
		#flash_signal(0)
		if get_f('qs_autokill_temple'):
			lnk(xx = 484, yy = 481, name_id = [14677, 14733, 14725, 14724, 14726])
			loot_items(xx = 484, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076])




##################################
###  TEMPLE OF ELEMENTAL EVIL	 #
##################################


	if (cur_map == 5111): # Tower Sentinel
		if get_f('qs_autokill_temple'):
			lnk(xx = 480, yy = 490, name_id = 14157)
			loot_items(xx = 480, yy = 490)

	if (cur_map == 5065): # Brigand Tower
		if get_f('qs_autokill_temple'):
			lnk(xx = 477, yy = 490, name_id = [14314, 14313, 14312, 14310, 14424, 14311, 14425])
			lnk(xx = 490, yy = 480, name_id = [14314, 14313, 14312, 14310, 14424, 14311, 14425])

			loot_items(item_proto_list = [10005, 6051], item_autoconvert_list = [4081, 6398, 4067])
			loot_items(xx = 490, yy = 480, item_proto_list = [10005, 6051], item_autoconvert_list = [4081, 6398, 4067, 4070, 4117, 5011])


	if (cur_map == 5066): # Temple Level 1 - Earth Floor
		if get_f('qs_autokill_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				
				#Stirges
				lnk(xx = 415, yy = 490, name_id = [14182])

				# Harpies & Ghouls
				lnk(xx = 418, yy = 574, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 401, yy = 554, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 401, yy = 554, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 421, yy = 544, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 413, yy = 522, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				loot_items(xx = 401, yy = 554)


				# Gel Cube + Grey Ooze
				lnk(xx = 407, yy = 594, name_id = [14095, 14129, 14139, 14140])
				loot_items(xx = 407, yy = 600, loot_source_name = [14448, 1049], item_autoconvert_list = [4121, 4118, 4113, 4116, 5005, 5098])


				# Corridor Ghouls
				lnk(xx = 461, yy = 600, name_id = [14095, 14129])
				
				# Corridor Gnolls
				lnk(xx = 563, yy = 600, name_id = [14078, 14079, 14080])
				loot_items(xx = 563, yy = 600, loot_source_name = [14078, 14079, 14080, 1049])


				# Corridor Ogre
				lnk(xx = 507, yy = 600, name_id = [14448])
				loot_items(xx = 507, yy = 600, loot_source_name = [14448, 1049], item_autoconvert_list = [4121, 4118, 4113, 4116, 5005, 5098])
			
				# Bone Corridor Undead
				lnk(xx = 497, yy = 519, name_id = [14107, 14081, 14082])
				lnk(xx = 467, yy = 519, name_id = [14083, 14107, 14081, 14082])
				loot_items(xx = 507, yy = 600, loot_source_name = [14107, 14081, 14082])

				# Wonnilon Undead
				lnk(xx = 536, yy = 414, name_id = [14127, 14126, 14125, 14124, 14092, 14123])
				lnk(xx = 536, yy = 444, name_id = [14127, 14126, 14125, 14124, 14092, 14123])


				# Huge Viper
				lnk(xx = 550, yy = 494, name_id = [14088])

				# Ogre + Goblins
				lnk(xx = 565, yy = 508, name_id = [14185, 14186, 14187, 14448])
				lnk(xx = 565, yy = 494, name_id = [14185, 14186, 14187, 14448])
				loot_items(xx = 565, yy = 508, loot_source_name = [14185, 14186, 14187, 14448])

				# Ghasts near prisoners
				lnk(xx = 545, yy = 553, name_id = [14128, 14129, 14136, 14095, 14137, 14135])
				loot_items(xx = 545, yy = 553, loot_source_name = [1040])

				# Black Widow Spiders
				lnk(xx = 440, yy = 395, name_id = [14417])


				# NW Ghast room near hideout
				lnk(xx = 390, yy = 390, name_id = [14128, 14129, 14136, 14095, 14137, 14135])


				if get_v('qs_autokill_temple_level_1_stage') == 0:
					set_v('qs_autokill_temple_level_1_stage', 1)
					
				elif get_v('qs_autokill_temple_level_1_stage') == 1:
					set_v('qs_autokill_temple_level_1_stage', 2)

					# Gnoll & Bugbear southern room
					lnk(xx = 515, yy = 535, name_id = [14078, 14249, 14066, 14632, 14164])
					lnk(xx = 515, yy = 549, name_id = [14067, 14631, 14078, 14249, 14066, 14632, 14164])
					loot_items(xx = 515, yy = 540)

					# Gnoll & Bugbear northern room
					lnk(xx = 463, yy = 535, name_id = [14248, 14631, 14188, 14636, 14083, 14184, 14078, 14249, 14066, 14632, 14164])
					loot_items(xx = 463, yy = 535)

					# Earth Temple Fighter eastern room
					lnk(xx = 438, yy = 505, name_id = [14337, 14338])
					loot_items(xx = 438, yy = 505, item_autoconvert_list = [6074, 6077, 5005, 4123, 4134])

					# Bugbear Central Outpost
					lnk(xx = 505, yy = 476, name_id = [14165, 14163, 14164, 14162])
					loot_items(xx = 505, yy = 476)

					# Bugbears nea r Wonnilon
					lnk(xx = 555, yy = 436, name_id = [14165, 14163, 14164, 14162])
					lnk(xx = 555, yy = 410, name_id = [14165, 14163, 14164, 14162])
					lnk(xx = 519, yy = 416, name_id = [14165, 14163, 14164, 14162])

					loot_items(xx = 519, yy = 416, loot_source_name = range(14162, 14166), item_autoconvert_list = [6174])
					loot_items(xx = 555, yy = 436, loot_source_name = [14164], item_autoconvert_list = [6174])
					loot_items(xx = 555, yy = 410, loot_source_name = [14164], item_autoconvert_list = [6174])

					# Bugbears North of Romag
					lnk(xx = 416, yy = 430, name_id = range(14162, 14166) )
					loot_items(xx = 416, yy = 430, loot_source_name = range(14162, 14166), item_autoconvert_list = [6174])

				elif get_v('qs_autokill_temple_level_1_stage') == 2:
					# Jailer room
					lnk(xx = 568, yy = 462, name_id = [14165, 14164, 14229])
					loot_items(xx = 568, yy = 462, item_autoconvert_list = [6174])
					# Earth Altar
					lnk(xx = 474, yy = 396, name_id = [14381, 14337])
					lnk(xx = 494, yy = 396, name_id = [14381, 14337])
					lnk(xx = 484, yy = 423, name_id = [14296])
					loot_items(xx = 480, yy = 400, loot_source_name = range(1041, 1045), item_proto_list = [6082, 12228, 12031] , item_autoconvert_list = [4070, 4193, 6056, 8025])
					loot_items(xx = 480, yy = 400, item_proto_list = [6082, 12228, 12031] , item_autoconvert_list = [4070, 4193, 6056, 8025])


					# Troop Commander room
					lnk(xx = 465, yy = 477, name_id = ( range(14162, 14166)+ [14337, 14156, 14339]) )
					lnk(xx = 450, yy = 477, name_id = ( range(14162, 14166)+ [14337, 14156, 14339]) )
					loot_items(xx = 450, yy = 476, item_autoconvert_list = [4098, 6074, 6077, 6174])


					# Romag Room
					lnk(xx = 441, yy = 442, name_id = ([8045, 14154] + range(14162, 14166)+ [14337, 14156, 14339]) )
					loot_items(xx = 441, yy = 442, item_autoconvert_list = [6164, 9359, 8907, 9011], item_proto_list = [10006, 6094, 4109, 8008])
					

	if (cur_map == 5067): # Temple Level 2 - Water, Fire & Air Floor
		if get_f('qs_autokill_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Kelno regroup
				lnk(xx = 480, yy = 494, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				lnk(xx = 490, yy = 494, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				lnk(xx = 490, yy = 514, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				loot_items(xx = 480, yy = 494, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])
				loot_items(xx = 480, yy = 514, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])
				loot_items(xx = 490, yy = 514, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])

				# Corridor Ogres
				lnk(xx = 480, yy = 452, name_id = [14249, 14353])
				loot_items(xx = 480, yy = 452, item_autoconvert_list = [4134])

				# Minotaur
				for m_stat in game.obj_list_vicinity(location_from_axis(566, 408), OLC_SCENERY):
					if m_stat.name == 1615:
						m_stat.destroy()
						cnk(14241)
						loot_items(xx = 566, yy = 408)

				# Greater Temple Guards
				lnk(xx = 533, yy = 398, name_id = [14349, 14348])
				lnk(xx = 550, yy = 422, name_id = [14349, 14348])
				loot_items(xx = 533, yy = 398)

				# Littlest Troll
				lnk(xx = 471, yy = 425, name_id = [14350])
				# Carrion Crawler
				lnk(xx = 451, yy = 424, name_id = [14190])


				# Fire Temple Bugbears Outside
				lnk(xx = 397, yy = 460, name_id = [14169])
				loot_items(xx = 397, yy = 460, loot_source_name = [14169])


				if get_v('qs_autokill_temple_level_2_stage') == 0:
					set_v('qs_autokill_temple_level_2_stage', 1)
					
				elif get_v('qs_autokill_temple_level_2_stage') == 1:
					set_v('qs_autokill_temple_level_2_stage', 2)

					# Feldrin
					lnk(xx = 562, yy = 438, name_id = [14311, 14312, 14314, 8041, 14253])
					loot_items(xx = 562, yy = 438, item_proto_list = [6083, 10010, 4082, 6086, 8010], item_autoconvert_list = [6091, 4070, 4117, 4114, 4062, 9426, 8014])

					# Prisoner Guards - Ogre + Greater Temple Bugbear
					lnk(xx = 410, yy = 440, name_id = [8065])
					loot_items(xx = 410, yy = 440, loot_source_name = [8065])

				elif get_v('qs_autokill_temple_level_2_stage') == 2:
					set_v('qs_autokill_temple_level_2_stage', 3)

					# Water Temple
					lnk(xx = 541, yy = 573, name_id = [14375, 14231, 8091, 14247, 8028, 8027, 14181, 14046, 14239, 14225])
					# Juggernaut
					lnk(xx = 541, yy = 573, name_id = [14244])
					loot_items(xx = 541, yy = 573, item_proto_list = [10008, 6104, 4124, 6105, 9327, 9178], item_autoconvert_list = [6039, 9508, 9400, 6178, 6170, 9546, 9038, 9536])

					# Oohlgrist
					lnk(xx = 483, yy = 614, name_id = [14262, 14195])
					loot_items(xx = 483, yy = 614, item_proto_list = [6101, 6107], item_autoconvert_list = [6106, 12014, 6108])

					# Salamanders
					lnk(xx = 433, yy = 583, name_id = [8063, 14384, 14111])
					lnk(xx = 423, yy = 583, name_id = [8063, 14384, 14111])
					loot_items(xx = 433, yy = 583, item_proto_list = [4028, 12016, 6101, 4136], item_autoconvert_list = [6121, 8020])

				elif get_v('qs_autokill_temple_level_2_stage') == 3:
					set_v('qs_autokill_temple_level_2_stage', 4)

					# Alrrem
					lnk(xx = 415, yy = 499, name_id = [14169, 14211, 8047, 14168, 14212, 14167, 14166, 14344, 14224, 14343])
					loot_items(xx = 415, yy = 499, item_proto_list = [10007, 4079, 6082], item_autoconvert_list = [6094, 6060, 6062, 6068, 6069, 6335, 6269, 6074, 6077, 6093, 6167, 6177, 6172, 8019, 6039, 4131, 6050, 4077, 6311])

				elif get_v('qs_autokill_temple_level_2_stage') == 4:
					set_v('qs_autokill_temple_level_2_stage', 5)
					# Big Bugbear Room
					lnk(xx = 430, yy = 361, name_id = (range(14174, 14178) +[14213, 14214, 14215, 14216])  )
					lnk(xx = 430, yy = 391, name_id = (range(14174, 14178) +[14213, 14214, 14215, 14216])  )
					loot_items(xx = 430, yy = 361, item_autoconvert_list = [6093, 6173, 6168, 6163, 6056])
					loot_items(xx = 430, yy = 391, item_autoconvert_list = [6093, 6173, 6168, 6163, 6056])


	if (cur_map == 5105): # Temple Level 3 - Thrommel Floor
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Northern Trolls
				lnk(xx = 394, yy = 401, name_id = [14262])

				# Shadows
				lnk(xx = 369, yy = 431, name_id = [14289])
				lnk(xx = 369, yy = 451, name_id = [14289])

				# Ogres:
				lnk(xx = 384, yy = 465, name_id = [14249])
				loot_items(xx = 384, yy = 465)

				# Ettin:
				lnk(xx = 437, yy =524, name_id = [14238])
				loot_items(xx = 437, yy = 524)

				# Yellow Molds:
				lnk(xx = 407, yy =564, name_id = [14276])

				# Groaning Spirit:
				lnk(xx = 441, yy = 459, name_id = [14280])
				loot_items(xx = 441, yy = 459, item_proto_list = [4218, 6090], item_autoconvert_list = [9214, 4191, 6058, 9123, 6214, 9492, 9391, 4002])

				# Key Trolls:
				lnk(xx = 489, yy = 535, name_id = [14262])
				lnk(xx = 489, yy = 504, name_id = [14262])
				loot_items(xx = 489, yy = 504, item_proto_list = range(10016, 10020) )
				loot_items(xx = 489, yy = 535, item_proto_list = range(10016, 10020) )

				# Will o Wisps:
				lnk(xx = 551, yy = 583, name_id = [14291])

				# Lamia:
				lnk(xx = 584, yy = 594, name_id = [14342, 14274])
				loot_items(xx = 584, yy = 594, item_proto_list = [4083])

				# Jackals, Werejackals & Gargoyles:
				lnk(xx = 511, yy = 578, name_id = [14051, 14239, 14138])
				lnk(xx = 528, yy = 556, name_id = [14051, 14239, 14138])

				# UmberHulks
				lnk(xx = 466, yy = 565, name_id = [14260])

				if get_v('qs_autokill_temple_level_3_stage') == 0:
					set_v('qs_autokill_temple_level_3_stage', 1)
					
				elif get_v('qs_autokill_temple_level_3_stage') == 1:
					set_v('qs_autokill_temple_level_3_stage', 2)
				
					# Gel Cube
					lnk(xx = 476, yy = 478, name_id = [14139])

					# Black Pudding
					lnk(xx = 442, yy = 384, name_id = [14143])

					# Goblins:
					lnk(xx = 491, yy = 389, name_id = (range(14183, 14188)+ [14219, 14217]) )
					loot_items(xx = 491, yy = 389)

					# Carrion Crawler:
					lnk(xx = 524, yy = 401, name_id = [14190] )

					# Ogres near thrommel:
					lnk(xx = 569, yy = 412, name_id = [14249, 14353] )
					loot_items(xx = 569, yy = 412, loot_source_name = [14249, 14353], item_autoconvert_list = [4134])

					# Leucrottas:
					lnk(xx = 405, yy = 590, name_id = [14351] )

				elif get_v('qs_autokill_temple_level_3_stage') == 2:
					set_v('qs_autokill_temple_level_3_stage', 3)

					# Pleasure dome:
					lnk(xx = 553, yy = 492, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 540, yy = 480, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 569, yy = 485, name_id = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177] )



					loot_items(xx = 540, yy = 480, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					loot_items(xx = 553, yy = 492, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					loot_items(xx = 569, yy = 485, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					game.global_flags[164] = 1 # Turns on Bugbears

				elif get_v('qs_autokill_temple_level_3_stage') == 3:
					set_v('qs_autokill_temple_level_3_stage', 4)
					# Pleasure dome - make sure:
					lnk(xx = 553, yy = 492, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 540, yy = 480, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 569, yy = 485, name_id = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177] )


					# Smigmal & Falrinth
					ass1 = game.obj_create(14782, location_from_axis(614, 455) )
					ass2 = game.obj_create(14783, location_from_axis(614, 455) )
					lnk(xx = 614, yy = 455, name_id = [14232, 14782, 14783] )
					loot_items(xx = 614, yy = 455, item_proto_list = [10011, 6125, 6088], item_autoconvert_list = [4126, 6073, 6335, 8025])

					lnk(xx = 614, yy = 480, name_id = [14110, 14177, 14346, 20123] )
					loot_items(xx = 619, yy = 480, item_proto_list = [12560, 10012, 6119], item_autoconvert_list = [4179, 9173])
					loot_items(xx = 612, yy = 503, loot_source_name = [1033], item_proto_list = [12560, 10012, 6119], item_autoconvert_list = [4179, 9173])


	if (cur_map == 5080): # Temple Level 4 - Greater Temple
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				game.global_flags[820] = 1 # Trap Disabled
				game.global_flags[148] = 1 # Paida Sane
				# Eastern Trolls
				lnk(xx = 452, yy = 552, name_id = [14262])
				# Western Trolls
				lnk(xx = 513, yy = 552, name_id = [14262])

				# Troll + Ettin
				lnk(xx = 522, yy = 586, name_id = [14262, 14238])
				loot_items(xx = 522, yy = 586)

				# Hill Giants
				lnk(xx = 570, yy = 610, name_id = [14218, 14217, 14219])
				loot_items(xx = 570, yy = 610)

				# Ettins
				lnk(xx = 587, yy = 580, name_id = [14238])
				loot_items(xx = 587, yy = 580)

				# More Trolls
				lnk(xx = 555, yy = 546, name_id = [14262])

				if get_v('qs_autokill_temple_level_4_stage') == 0:
					set_v('qs_autokill_temple_level_4_stage', 1)
					
				elif get_v('qs_autokill_temple_level_4_stage') == 1:
					set_v('qs_autokill_temple_level_4_stage', 2)
					# Bugbear quarters
					lnk(xx = 425, yy = 591, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 435, yy = 591, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 434, yy = 603, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 405, yy = 603, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])

					loot_items(xx = 435, yy = 590)
					loot_items(xx = 425, yy = 590)
					loot_items(xx = 435, yy = 603)
					loot_items(xx = 405, yy = 603)

				elif get_v('qs_autokill_temple_level_4_stage') == 2:
					set_v('qs_autokill_temple_level_4_stage', 3)
					# Insane Ogres
					lnk(xx = 386, yy = 584, name_id = [14356, 14355, 14354])
					loot_items(xx = 386, yy = 584)
					# Senshock's Posse
					lnk(xx = 386, yy = 528, name_id = [14296, 14298, 14174, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(386, 528), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 386, yy = 528)


				elif get_v('qs_autokill_temple_level_4_stage') == 3:
					set_v('qs_autokill_temple_level_4_stage', 4)
					# Hedrack's Posse
					lnk(xx = 493, yy = 442, name_id = [14238, 14239, 14218, 14424, 14296, 14298, 14174, 14176, 14177, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(493, 442), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 493, yy = 442)

					lnk(xx = 465, yy = 442, name_id = [14238, 14239, 14218, 14424, 14296, 14298, 14174, 14176, 14177, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(493, 442), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 493, yy = 442)

					# Fungi
					lnk(xx = 480, yy = 375, name_id = [14274, 14143, 14273, 14276, 14142, 14141, 14282])
					loot_items(xx = 484, yy = 374)
					loot_items(xx = 464, yy = 374)

					lnk(xx = 480, yy = 353, name_id = [14277, 14140])





##################################
###  NODES						 #
##################################

	if (cur_map == 5083): # Fire Node
		if get_f('qs_autokill_nodes'):
			# Fire Toads
			lnk(xx = 535, yy = 525, name_id = [14300])
			
			# Bodaks
			lnk(xx = 540, yy = 568, name_id = [14328])
			
			# Salamanders
			lnk(xx = 430, yy = 557, name_id = [14111])
			
			# Salamanders near Balor
			lnk(xx = 465, yy = 447, name_id = [14111])			
			
			# Efreeti
			lnk(xx = 449, yy = 494, name_id = [14340])

			# Fire Elementals + Snakes
			lnk(xx = 473, yy = 525, name_id = [14298, 14626])
			lnk(xx = 462, yy = 532, name_id = [14298, 14626])

				





		


			




##########################
###  VERBOBONC		 #
##########################

	if (cur_map == 5154): # Scarlett Bro bottom floor
		if get_f('qs_autokill_greater_temple'):
			game.global_flags[984] = 1 # Skip starter convo
			game.global_flags[982] = 1


	if (cur_map == 5152): # Prince Zook quarters
		if get_f('qs_autokill_greater_temple'):
			game.global_flags[969] = 1 # Met prince Zook
			game.global_flags[985] = 1 # Mention Drow Problem
			game.quests[69].state = qs_accepted
			game.global_flags[981] = 1 # Zook said Lerrick mean
			game.global_vars[977] = 1 # Zook said talk to Absalom abt Lerrick
			if game.global_vars[999]  >= 15:
				game.quests[69].state = qs_completed
			

	if (cur_map == 5126): # Drow Caves I - spidersfest
		if get_f('qs_autokill_greater_temple'):
			
			# Spidors 1
			lnk(xx = 465, yy = 471, name_id = [14399, 14397])
			lnk(xx = 451, yy = 491, name_id = [14399, 14397])
			lnk(xx = 471, yy = 491, name_id = [14399, 14397])

			lnk(xx = 437, yy = 485, name_id = [14741, 14397])
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Key
				loot_items(item_proto_list = [10022], loot_money_and_jewels_also = 0)

		return



	if (cur_map == 5127): # Drow Caves II - 2nd spidersfest
		if get_f('qs_autokill_greater_temple'):			

			# Spiders
			lnk(xx = 488, yy = 477, name_id = [14741, 14397, 14620])

			# Drow
			lnk(xx = 455, yy = 485, name_id = [14708, 14737, 14736, 14735])
			loot_items(xx = 455, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073])

	if (cur_map == 5128): # Drow Caves III - Drowfest I
		if get_f('qs_autokill_greater_temple'):			

			# Garg. Spider
			lnk(xx = 497, yy = 486, name_id = [14524])

			# Drow
			lnk(xx = 473, yy = 475, name_id = [14399, 14708, 14737, 14736, 14735])
			lnk(xx = 463, yy = 485, name_id = [14399, 14708, 14737, 14736, 14735])
			loot_items(xx = 475, yy = 471, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073])

			lnk(xx = 456, yy = 487, name_id = [14399, 14708, 14737, 14736, 14735, 14734])
			lnk(xx = 427, yy = 487, name_id = [14399, 14708, 14737, 14736, 14735, 14734])
			loot_items(xx = 465, yy = 486, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])
			loot_items(xx = 425, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])
			loot_items(xx = 475, yy = 471, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])

			loot_items(xx = 425, yy = 481, item_proto_list = [6051, 4139, 4137] )

	if (cur_map == 5129): # Drow Caves IV - Spiders cont'd
		if get_f('qs_autokill_greater_temple'):			
			lnk(xx = 477, yy = 464, name_id = [14524, 14399, 14397])
			lnk(xx = 497, yy = 454, name_id = [14524, 14399, 14397])
			lnk(xx = 467, yy = 474, name_id = [14524, 14399, 14397, 14741])
			lnk(xx = 469, yy = 485, name_id = [14524, 14399, 14397])


	if (cur_map == 5130): # Drow Caves V - Young White Dragons
		if get_f('qs_autokill_greater_temple'):	
			lnk(xx = 489, yy = 455, name_id = [14707])

	if (cur_map == 5131): # Drow Caves VI - Adult White Dragon
		if get_f('qs_autokill_greater_temple'):	
			lnk(xx = 480, yy = 535, name_id = [14999])
			loot_items(xx = 480, yy = 535)


	if (cur_map == 5148): # Verbobonc Jail
		if get_f('qs_autokill_greater_temple'):	
			game.quests[79].state = qs_accepted
			game.quests[80].state = qs_accepted
			game.quests[81].state = qs_accepted
			if game.global_vars[964] == 0:
				game.global_vars[964] = 1
			if game.global_flags[956] == 1:
				game.quests[79].state = qs_completed
			if game.global_flags[957] == 1:
				game.quests[80].state = qs_completed
			if game.global_flags[958] == 1:
				game.quests[81].state = qs_completed

	if (cur_map == 5151): # Verbobonc Great Hall
		if get_f('qs_autokill_greater_temple'):	
			game.global_vars[979] = 2 # Allows meeting with Mayor
			game.global_flags[980] = 1 # Got info about Verbobonc

	if (cur_map == 5124): # Spruce Goose Inn
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				lnk(xx=484, yy = 479, name_id = 8716) # Guntur Gladstone
				game.global_vars[961] = 2 # Have discussed wreaking havoc
				loot_items(loot_source_name = 8716, item_autoconvert_list = [6202, 6306, 4126, 4161])




	return





#######################
#######################
### END OF AUTOKILL ###
#######################
#######################



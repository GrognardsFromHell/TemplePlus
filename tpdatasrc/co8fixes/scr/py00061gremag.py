from toee import *
from utilities import *
from InventoryRespawn import *
from py00439script_daemon import record_time_stamp, get_v, set_v, tsc, within_rect_by_corners
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):

	assassin_of_lodriss = triggerer.reputation_has(21) == 1
	butcher_of_hommlet = triggerer.reputation_has(1) == 1
	is_rogue = attachee.stat_level_get(stat_level_rogue) >= 2
	is_evil = ( triggerer.stat_level_get(stat_alignment) & ALIGNMENT_EVIL ) != 0
	is_friend_of_lareth = triggerer.reputation_has(18) == 1
	is_beggar_maker = triggerer.reputation_has(5) == 1
	
	if (attachee.map == 5074): # In the campsite
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog( attachee, 800)
	# double crossed (need to add flag for actually getting a reward)
	
		
	#turned R&G in to Burne and confronted them about the spy as well:
	if (game.quests[16].state == qs_completed and game.quests[15].state == qs_completed): 
		# received compensation for promising not to tell
		triggerer.begin_dialog( attachee, 20 )
	
	#suggest assassination quests
	elif (game.global_flags[293] == 0 and attachee.has_met(triggerer)) and ( assassin_of_lodriss or butcher_of_hommlet or is_friend_of_lareth or ( is_rogue and is_evil ) or (is_beggar_maker and is_evil) ):
		triggerer.begin_dialog( attachee, 290 )
	#Ask about Lareth's whereabouts
	elif ((game.global_flags[835] == 1 or game.global_flags[837] == 1) and game.global_flags[37] == 0 and attachee.has_met(triggerer) and not game.quests[16].state == qs_completed and not game.quests[15].state == qs_completed and game.global_flags[843] == 0):
		triggerer.begin_dialog( attachee, 2500 )

	#Confronted about spy or courier, but have not ratted to Burne yet
	elif (game.global_flags[41] == 1 or game.quests[16].state == qs_completed or game.quests[64].state == qs_completed): 
		triggerer.begin_dialog( attachee, 260 )
	#Courier has spilled guts, Gremag now wary
	elif (game.quests[17].state == qs_completed or game.quests[64].state == qs_botched):
		triggerer.begin_dialog( attachee, 30 )
	#Exposed laborer spy
	elif (game.global_flags[31] == 1):
		triggerer.begin_dialog( attachee, 40 )
	#None of the above, but Man-At-Arms hired
	elif (game.global_flags[39] == 1):
		triggerer.begin_dialog( attachee, 50 )
	#completed 1st assassination
	elif (game.global_flags[297] == 1 and game.quests[91].state == qs_accepted):
		triggerer.begin_dialog( attachee, 390)
	#completed 2nd assassination
	elif (game.global_flags[298] == 1 and game.quests[92].state == qs_accepted):
		triggerer.begin_dialog( attachee, 400)
	#completed 3rd assassination
	elif (game.global_flags[299] == 1 and game.quests[93].state == qs_accepted):
		triggerer.begin_dialog( attachee, 410)
	elif (attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 500 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5010):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		elif (game.global_vars[751] == 0):
			attachee.object_flag_unset(OF_OFF)
			if (game.global_flags[907] == 0):
				game.timevent_add(respawn, (attachee), 86400000 ) #86400000ms is 24 hours
				game.global_flags[907] = 1
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[815] = 1
	if (game.global_flags[814] == 1):
		for pc in game.party:
			if ( pc.reputation_has( 23 ) == 1):
				pc.reputation_remove( 23 )
	if (game.party[0].reputation_has(9) == 0):
		game.party[0].reputation_add(9)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (triggerer.type == obj_t_pc and game.global_vars[751] == 0):
		raimol = find_npc_near( attachee, 8050)
		if (raimol != OBJ_HANDLE_NULL):
			attachee.float_line(280,triggerer)
			leader = raimol.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				leader.follower_remove(raimol)
			raimol.attack(triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (game.global_vars[751] == 1 and attachee.map == 5010 and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		game.global_vars[751] = 2
		create_item_in_inventory( 8010, attachee )
		return RUN_DEFAULT
	if (game.global_vars[751] == 2 and attachee.map == 5010 and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		game.global_vars[751] = 3
		if (game.party[0].reputation_has(23) == 0):
			game.party[0].reputation_add(23)
		attachee.runoff(attachee.location-3)
		return SKIP_DEFAULT
	if ( game.global_vars[751] == 0 and attachee.stat_level_get(stat_hp_current) >= 0 and game.global_flags[814] == 1 and attachee.map == 5010)  and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**10 == 0):
		found_pc = OBJ_HANDLE_NULL
		rannos = find_npc_near(attachee,8048)
		raimol = find_npc_near(attachee,8050)
		for pc in game.party[0].group_list():
			if pc.type == obj_t_pc and pc.is_unconscious() == 0:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
				rannos.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
			else:
				attachee.ai_shitlist_remove( pc )
				rannos.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
				pc.ai_shitlist_remove( attachee )
				pc.ai_shitlist_remove( rannos )
				pc.ai_shitlist_remove( raimol )
		if (found_pc != OBJ_HANDLE_NULL):
			found_pc.begin_dialog( attachee, 1100 )
			return SKIP_DEFAULT
	if ( obj_percent_hp(attachee) < 95 and game.global_vars[751] == 0 and attachee.stat_level_get(stat_hp_current) >= 0 and attachee.map == 5010)  and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**10 == 0):
		found_pc = OBJ_HANDLE_NULL
		rannos = find_npc_near(attachee,8048)
		raimol = find_npc_near(attachee,8050)
		for pc in game.party[0].group_list():
			if pc.type == obj_t_pc and pc.is_unconscious() == 0:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
				rannos.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
			else:
				attachee.ai_shitlist_remove( pc )
				rannos.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
				pc.ai_shitlist_remove( attachee )
				pc.ai_shitlist_remove( rannos )
				pc.ai_shitlist_remove( raimol )
		if (found_pc != OBJ_HANDLE_NULL):
			if (game.global_flags[814] == 0 and rannos.stat_level_get(stat_hp_current) >= 0):
				found_pc.begin_dialog( attachee, 1000 )
				return SKIP_DEFAULT
			if (game.global_flags[814] == 1 or rannos.stat_level_get(stat_hp_current) <= -1):
				found_pc.begin_dialog( attachee, 1100 )
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
	game.global_flags[815] = 0
	if (game.party[0].reputation_has(9) == 1):
		for pc in game.party:
			if ( pc.reputation_has( 23 ) == 0):
				pc.reputation_add( 23 )
		game.party[0].reputation_remove(9)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	itemA = attachee.item_find(8010)
	if (itemA != OBJ_HANDLE_NULL and game.global_vars[751] == 0 and attachee.map == 5010):
		itemA.destroy()
#		create_item_in_inventory( 8021, attachee )
		game.new_sid = 0
	return RUN_DEFAULT


def switch_to_rannos( gremag, pc ):
	rannos = find_npc_near(gremag,8048)
	game.global_vars[750] = 1
	game.global_vars[751] = 1
	rannos.turn_towards(gremag)
	gremag.turn_towards(rannos)
	pc.begin_dialog(rannos,1010)
	return SKIP_DEFAULT


def respawn(attachee):
	box = find_container_near(attachee,1004)
	RespawnInventory(box)
	game.timevent_add(respawn, (attachee), 86400000 ) #86400000ms is 24 hours
	return


def buff_npc( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14607 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_stoneskin, obj)
		if (obj.name == 14609 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_shield_of_faith, obj)
	return RUN_DEFAULT


def buff_npc_two( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14607 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_improved_invisibility, obj)
		if (obj.name == 14609 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_owls_wisdom, obj)
	return RUN_DEFAULT


def buff_npc_four( attachee, triggerer ):
	target = find_npc_near( attachee, 14606)
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14607 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, attachee)
		if (obj.name == 14609 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, target)
	return RUN_DEFAULT


def ishurt(attachee, percent):
	maxhp = attachee.stat_level_get(stat_hp_max)
	curhp = attachee.stat_level_get(stat_hp_current)
	subdam = attachee.stat_level_get(stat_subdual_damage)
	nordam = maxhp - curhp
	percent_hurt = (nordam + subdam) * 100 / maxhp
	if percent_hurt > percent:
		return 1
	else:
		return 0


def lawful_ok():
	#this script is used to determine if there's sufficient proof for a lawful party to attack R&G
	a = 0
	if (game.global_flags[31] == 1):
	#found out the laborer spy
		a = a + 1
	if (game.global_flags[428] == 1):
	#confronted about assassination attempt. only enabled if 292 == 1, so no need to check for it too
		a = a + 1
	if (game.quests[17].state == qs_completed):
	#found out the courier
		a = a + 1
	elif (game.quests[17].state != qs_unknown and game.global_flags[40] == 1 and game.global_flags[4] == 1):
	#found out about Corl poisoning sheep, and teamster has attested to strange goings on in the barn, and you've met the courier
		a = a + 1
	if (game.global_flags[423] == 1 and game.global_flags[292] == 1 and game.global_flags[40] == 1):
	#found out about Corl having been assassinated, connected it with other assassinations and strange behavior of the barn courier
		a = a + 1
	if (a > 1):
		return 1
	else:
		return 0


def is_lawful_party():
	if game.party_alignment == LAWFUL_NEUTRAL or game.party_alignment == LAWFUL_GOOD or game.party_alignment == LAWFUL_EVIL:
		return 1
	else:
		return 0


def party_lawful():
#yeah yeah duplicity blah blah
	if (game.party_alignment == LAWFUL_GOOD or game.party_alignment == LAWFUL_NEUTRAL or game.party_alignment == LAWFUL_EVIL):
		return 1
	else:
		return 0


def party_good():
	if (game.party_alignment == NEUTRAL_GOOD or game.party_alignment == TRUE_NEUTRAL or game.party_alignment == CHAOTIC_GOOD):
		return 1
	else:
		return 0


def party_bad():
	if (game.party_alignment == CHAOTIC_NEUTRAL or game.party_alignment == NEUTRAL_EVIL or game.party_alignment == CHAOTIC_EVIL):
		return 1
	else:
		return 0


def burne_quest():
	if game.quests[64].state == qs_accepted or game.quests[64].state == qs_mentioned:
		game.quests[64].state = qs_completed
	return


def rngfighttime_set():
	if game.global_flags[426] == 0:
		record_time_stamp(426)
		game.global_flags[426] = 1
	return


def traders_exposed():
	#this script determines whether the traders surmise they are about to be exposed
	#it uses the time stamps to determine whether they knew this stuff BEFORE you assaulted them
	a = 0
	if (game.quests[15].state == qs_completed and tsc( 427 , 426 ) == 1):
	#laborer spy revealed to Burne
		a = a + 1
	if (game.quests[16].state == qs_completed and tsc( 431 , 426 ) == 1 ):
	#confronted traders about laborer spy
		a = a + 1
	if game.global_flags[444] == 1 or game.global_flags[422] == 1 or  (game.global_flags[428] == 1 and (game.global_flags[7] == 1 or (game.time.time_game_in_seconds(game.time) >= game.global_vars[423] + 24*60*60 and game.global_flags[4] == 1) )):
	#confronted about assassination attempt, and either presented hard evidence or Corl was killed too
		a = a + 1
	if (game.quests[17].state == qs_completed and tsc( 430 , 426 ) == 1 ):
	#found out the courier
		a = a + 1
	return a

#on cool list: add check for paladin in party, make him fall if there isn't sufficient evidence


def q16():
	if game.quests[16].state == qs_accepted or game.quests[16].state == qs_mentioned:
		game.quests[16].state = qs_completed
		record_time_stamp(431)
	return


def f41():
	if game.global_flags[41] == 0:
		game.global_flags[41] = 1
		record_time_stamp(432)
	return
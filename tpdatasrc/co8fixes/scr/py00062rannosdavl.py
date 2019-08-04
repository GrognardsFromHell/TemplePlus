from toee import *
from utilities import *
from InventoryRespawn import *
from py00439script_daemon import record_time_stamp, get_v, set_v, tsc, within_rect_by_corners
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.map == 5074):
		# Found them in the wilderness
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog( attachee, 820)
		return SKIP_DEFAULT
	if ((game.global_flags[835] == 1 or game.global_flags[837] == 1) and game.global_flags[37] == 0 and attachee.has_met(triggerer) and not game.quests[16].state == qs_completed and not game.quests[15].state == qs_completed and game.global_flags[843] == 0):
		# Lareth Prisoner subplot node (enables to ask about master)
		triggerer.begin_dialog( attachee, 2500 )
	if (game.quests[16].state == qs_completed and game.quests[15].state == qs_completed): 
		# Ratted Traders to Burne
		triggerer.begin_dialog( attachee, 20 )
	elif (game.global_flags[41] == 1 or game.quests[16].state == qs_completed or game.quests[64].state == qs_completed): 
		triggerer.begin_dialog( attachee, 290 )
	elif (game.quests[17].state == qs_completed):
		triggerer.begin_dialog( attachee, 30 )
	elif (game.global_flags[31] == 1 or game.quests[64].state == qs_botched):
		triggerer.begin_dialog( attachee, 50 )
	elif (attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 70 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5010):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		elif (game.global_vars[750] == 0):
			attachee.object_flag_unset(OF_OFF)
			if (game.global_flags[907] == 0):
				game.timevent_add(respawn, (attachee), 86400000 ) #86400000ms is 24 hours
				game.global_flags[907] = 1
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.map == 5010):
		rngfighttime_set()
		game.global_flags[426] = 1
	game.global_flags[814] = 1
	if (game.global_flags[815] == 1):
		for pc in game.party:
			if ( pc.reputation_has( 23 ) == 1):
				pc.reputation_remove( 23 )
	if (game.party[0].reputation_has(9) == 0):
		game.party[0].reputation_add(9)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (triggerer.type == obj_t_pc and game.global_vars[750] == 0):
		raimol = find_npc_near( attachee, 8050)
		if (raimol != OBJ_HANDLE_NULL):
			attachee.float_line(380,triggerer)
			leader = raimol.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				leader.follower_remove(raimol)
			raimol.attack(triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (game.global_vars[750] == 1 and attachee.map == 5010 and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		game.global_vars[750] = 2
		create_item_in_inventory( 8010, attachee )
		return RUN_DEFAULT
	if (game.global_vars[750] == 2 and attachee.map == 5010 and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		game.global_vars[750] = 3
		if (game.party[0].reputation_has(23) == 0):
			game.party[0].reputation_add(23)
		attachee.runoff(attachee.location-3)
		return SKIP_DEFAULT
	if ( game.global_vars[751] == 0 and attachee.stat_level_get(stat_hp_current) >= 0 and game.global_flags[815] == 1 and attachee.map == 5010)  and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**10 == 0):
		found_pc = OBJ_HANDLE_NULL
		gremag = find_npc_near(attachee,8049)
		raimol = find_npc_near(attachee,8050)
		for pc in game.party[0].group_list():
			if pc.type == obj_t_pc and pc.is_unconscious() == 0:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
				gremag.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
			else:
				attachee.ai_shitlist_remove( pc )
				gremag.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
				pc.ai_shitlist_remove( attachee )
				pc.ai_shitlist_remove( gremag )
				pc.ai_shitlist_remove( raimol )
		if (found_pc != OBJ_HANDLE_NULL):
			found_pc.begin_dialog( attachee, 1100 )
			return SKIP_DEFAULT
	if ( obj_percent_hp(attachee) < 95 and game.global_vars[750] == 0 and attachee.stat_level_get(stat_hp_current) >= 0 and attachee.map == 5010)  and (game.global_vars[450] & 2**0 == 0) and (game.global_vars[450] & 2**10 == 0):
		found_pc = OBJ_HANDLE_NULL
		gremag = find_npc_near(attachee,8049)
		raimol = find_npc_near(attachee,8050)
		for pc in game.party[0].group_list():
			if pc.type == obj_t_pc and pc.is_unconscious() == 0:
				found_pc = pc
				attachee.ai_shitlist_remove( pc )
				gremag.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
			else:
				attachee.ai_shitlist_remove( pc )
				gremag.ai_shitlist_remove( pc )
				raimol.ai_shitlist_remove( pc )
				pc.ai_shitlist_remove( attachee )
				pc.ai_shitlist_remove( gremag )
				pc.ai_shitlist_remove( raimol )
		if (found_pc != OBJ_HANDLE_NULL):
			if (game.global_flags[815] == 0 and gremag.stat_level_get(stat_hp_current) >= 0):
				found_pc.begin_dialog( attachee, 1000 )
				return SKIP_DEFAULT
			if (game.global_flags[815] == 1 or gremag.stat_level_get(stat_hp_current) <= -1):
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
	game.global_flags[814] = 0
	if (game.party[0].reputation_has(9) == 1):
		for pc in game.party:
			if ( pc.reputation_has( 23 ) == 0):
				pc.reputation_add( 23 )
		game.party[0].reputation_remove(9)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	itemA = attachee.item_find(8010)
	if (itemA != OBJ_HANDLE_NULL and game.global_vars[750] == 0 and attachee.map == 5010):
		itemA.destroy()
#		create_item_in_inventory( 8021, attachee )
		game.new_sid = 0
	return RUN_DEFAULT


def switch_to_gremag( rannos, pc ):
	gremag = find_npc_near(rannos,8049)
	game.global_vars[750] = 1
	game.global_vars[751] = 1
	pc.begin_dialog(gremag,1010)
	rannos.turn_towards(gremag)
	gremag.turn_towards(rannos)
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


def buff_npc_three( attachee, triggerer ):
	target = find_npc_near( attachee, 8049)
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14607 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, attachee)
		if (obj.name == 14609 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, target)
	return RUN_DEFAULT


def buff_npc_four( attachee, triggerer ):
	target = find_npc_near( attachee, 14606)
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14607 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, attachee)
		if (obj.name == 14609 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endurance, target)
	return RUN_DEFAULT


def burne_quest():
	if game.quests[64].state == qs_accepted or game.quests[64].state == qs_mentioned:
		game.quests[64].state = qs_completed
	return


def rngfighttime_set():
	if game.global_flags[426] == 0:
		record_time_stamp(426)
		game.global_flags[426] = 1
	return


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
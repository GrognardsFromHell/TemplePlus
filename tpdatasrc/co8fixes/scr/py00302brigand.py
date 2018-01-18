from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	game.new_sid = 0
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (game.global_flags[833] == 0 and attachee.map == 5065):
		for pc in game.party:
			if (pc.type == obj_t_pc):
				attachee.ai_shitlist_remove( pc )
		for obj in game.obj_list_vicinity(attachee.location,OLC_CRITTERS):
			if (obj.name == 8002 and obj.leader_get() != OBJ_HANDLE_NULL ):
				leader = obj.leader_get()
				leader.begin_dialog( obj, 266 )
		if (attachee.name == 14310 and game.global_flags[37] == 1 and game.global_flags[835] == 0): ## Lareth dead
			leader = game.party[0]
			if (leader.skill_level_get(attachee, skill_bluff) >= 10):
				leader.begin_dialog( attachee, 266 )
			elif (leader.skill_level_get(attachee, skill_bluff) >= 5):
				leader.begin_dialog( attachee, 66 )
			else:
				leader.begin_dialog( attachee, 166 )
		if (attachee.name == 14310 and game.global_flags[834] == 1 and game.global_flags[37] == 0 and game.global_flags[835] == 0): ## Lareth alive
			leader = game.party[0]
			leader.begin_dialog( attachee, 366 )
		if (attachee.name == 14310 and game.global_flags[834] == 0 and game.global_flags[37] == 0 and game.global_flags[835] == 0): ## Lareth not met
			leader = game.party[0]
			leader.begin_dialog( attachee, 466 )
		return SKIP_DEFAULT
	if (game.global_flags[839] == 1 and attachee.map == 5065 and game.global_flags[840] == 0):
		for pc in game.party:
			if (pc.type == obj_t_pc):
				attachee.ai_shitlist_remove( pc )
		for obj in game.obj_list_vicinity(attachee.location,OLC_CRITTERS):
			if (obj.name == 14614 and obj.leader_get() != OBJ_HANDLE_NULL ):
				leader = obj.leader_get()
				leader.begin_dialog( obj, 400 )
				return SKIP_DEFAULT
		if (game.global_flags[847] == 1):
			target = game.obj_create( 14617, location_from_axis (479L, 489L))
			game.global_flags[841] = 0
			game.global_flags[847] = 0
			target.turn_towards(game.party[0])
			game.party[0].begin_dialog(target, 350)
		return SKIP_DEFAULT
	if (game.global_flags[840] == 1 and attachee.map == 5065):
#		game.global_flags[840] = 0
		game.global_flags[849] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if attachee.map == 5002: # moathouse courtyard
		for npc in game.obj_list_vicinity(location_from_axis(477, 463), OLC_NPC):
			if npc.name == 14070 and npc.leader_get() == OBJ_HANDLE_NULL and npc.is_unconscious() == 0:
				npc.attack(game.leader)

	if ((game.global_flags[833] == 0 and attachee.map == 5065 and game.global_flags[835] == 0 and game.global_flags[849] == 0) or (game.global_flags[839] == 1 and attachee.map == 5065 and game.global_flags[840] == 0 and game.global_flags[849] == 0)):
		return SKIP_DEFAULT

	## THIS IS USED FOR BREAK FREE
	#found_nearby = 0
	#for obj in game.party[0].group_list():
	#	if (obj.distance_to(attachee) <= 3 and obj.stat_level_get(stat_hp_current) >= -9):
	#		found_nearby = 1
	#if found_nearby == 0:
	#	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
	#		attachee.item_find(8903).destroy()
	#	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	#	create_item_in_inventory( 8903, attachee )
##		attachee.d20_send_signal(S_BreakFree)

	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if attachee.map == 5002: # moathouse courtyard brigands
		attachee.scripts[15] = 302
	if (game.global_flags[841] == 1 and attachee.map == 5065 and attachee.name == 14310):
		target = find_npc_near( attachee, 14614)
		if (target == OBJ_HANDLE_NULL):
			target = game.obj_create( 14617, location_from_axis (479L, 489L))
			game.global_flags[841] = 0
			return RUN_DEFAULT		
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_flags[840] == 1 and attachee.map == 5065):
		return SKIP_DEFAULT
	if attachee.map == 5091:
		return SKIP_DEFAULT
	return RUN_DEFAULT


def buff_npc( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14424 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.turn_towards(attachee)
			obj.cast_spell(spell_mage_armor, obj)
		if (obj.name == 14425 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.turn_towards(attachee)
			obj.cast_spell(spell_shield_of_faith, obj)
	return RUN_DEFAULT

def buff_npc_two( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14424 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_mirror_image, obj)
		if (obj.name == 14425 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_owls_wisdom, obj)
	return RUN_DEFAULT

def buff_npc_three( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.name == 14424 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_protection_from_arrows, obj)
		if (obj.name == 14425 and obj.leader_get() == OBJ_HANDLE_NULL):
			obj.cast_spell(spell_endure_elements, obj)
	return RUN_DEFAULT
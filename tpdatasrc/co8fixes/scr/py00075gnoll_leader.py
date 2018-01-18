from toee import *
from utilities import *
from Co8 import Timed_Destroy
from combat_standard_routines import *
from py00439script_daemon import set_f, get_f


def san_dialog( attachee, triggerer ):
	if (game.global_flags[288] == 1 and game.global_flags[856] == 1 and game.global_flags[857] == 1):
		triggerer.begin_dialog( attachee, 200 )
	if (anyone( triggerer.group_list(), "has_follower", 8002 )):
		triggerer.begin_dialog( attachee, 100 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if attachee.map == 5005:
		for npc in game.obj_list_vicinity(attachee.location, OLC_NPC):
			if npc.name in [14067, 14078, 14079, 14080] and npc.leader_get() == OBJ_HANDLE_NULL and npc.scripts[15] == 0:
				npc.scripts[15] = 75
	if (game.global_vars[709] == 1 and attachee.map == 5005):
		game.global_vars[709] = 2
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14079 or obj.name == 14080 or obj.name == 14067 or obj.name == 14078 or obj.name == 14066):
				obj.turn_towards(triggerer)
				obj.attack(triggerer)
		game.new_sid = 0

	if (attachee.map == 5066):		#### change to ==
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14078 or obj.name == 14079 or obj.name == 14078 or obj.name == 14631 or obj.name == 14632 or obj.name == 14633 or obj.name == 14634 or obj.name == 14067 or obj.name == 14066 or obj.name == 14635):
				obj.turn_towards(triggerer)
				obj.attack(triggerer)
		game.new_sid = 0
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if attachee.map == 5005: # Moathouse Dungeon
		for npc in game.obj_list_vicinity(attachee.location, OLC_NPC):
			if npc.name in [14066, 14067, 14078, 14079, 14080] and npc.leader_get() == OBJ_HANDLE_NULL and npc.scripts[15] == 0:
				npc.scripts[15] = 75
	elif attachee.map == 5066: # Temple level 1
		for npc in game.obj_list_vicinity(attachee.location, OLC_NPC):
			if npc.leader_get() == OBJ_HANDLE_NULL and npc.scripts[15] == 0:
				npc.scripts[15] = 2
	if (game.global_vars[709] == 2 and attachee.map == 5005):
		game.global_vars[709] = 3
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14079 or obj.name == 14080 or obj.name == 14067 or obj.name == 14078 or obj.name == 14066):
				obj.turn_towards(triggerer)
				obj.attack(triggerer)
				
	if attachee.map == 5066 and (get_f('j_ogre_temple_1') == 0): # temple level 1 - gnolls near southern stairs
		xx,yy = location_to_axis( attachee.location )
		if ( (xx-564)**2 + (yy - 599)**2 ) < 200:
			set_f('j_ogre_temple_1')
			xxp_min = 564
			xxp_o = 564
			for pc in game.party[0].group_list():
				xxp, yyp = location_to_axis( pc.location )
				if yyp >= 599:
					if xxp < xxp_min:
						xxp_min = xxp
				elif yyp < 599 and xxp >= 519 and xxp <= 546 and yyp >= 589:
					if xxp < xxp_o:
						xxp_o = xxp
			x_ogre = min(xxp_min, xxp_o) - 20
			x_ogre = max( 507, x_ogre )
			for npc in game.obj_list_vicinity(location_from_axis(507, 603), OLC_NPC):
				if npc.name == 14448 and npc.leader_get() == OBJ_HANDLE_NULL and npc.is_unconscious() == 0:
					npc.move( location_from_axis( x_ogre , 601), 0, 0)
					npc.attack(game.leader)
				

			
			
## THIS IS USED FOR BREAK FREE
	found_nearby = 0
	for obj in game.party[0].group_list():
		if (obj.distance_to(attachee) <= 3 and obj.stat_level_get(stat_hp_current) >= -9):
			found_nearby = 1
	if found_nearby == 0:
		while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
			attachee.item_find(8903).destroy()
		#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
		#	create_item_in_inventory( 8903, attachee )
##		attachee.d20_send_signal(S_BreakFree)
	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[854] == 0 and attachee.map == 5066):
		game.global_flags[854] = 1
		newNPC = game.obj_create( 14459, location_from_axis (551L, 600L) )
		newNPC = game.obj_create( 14458, location_from_axis (511L, 540L) )
		newNPC = game.obj_create( 14452, location_from_axis (455L, 535L) )

	if (attachee.name == 14078 and attachee.leader_get() == OBJ_HANDLE_NULL and attachee.map == 5005 and game.global_vars[760] == 1):
		game.global_vars[760] = 2
		game.new_sid = 0
		return RUN_DEFAULT
	if (attachee.name == 14078 and attachee.leader_get() == OBJ_HANDLE_NULL and attachee.map == 5005):
		if (game.global_vars[760] == 2):
			attachee.standpoint_set( STANDPOINT_NIGHT, 272 )
			attachee.standpoint_set( STANDPOINT_DAY, 272 )
		if (game.global_vars[760] == 13):
			attachee.standpoint_set( STANDPOINT_NIGHT, 271 )
			attachee.standpoint_set( STANDPOINT_DAY, 271 )
		game.global_vars[760] = game.global_vars[760] + 1
		if (game.global_vars[760] >= 22):
			game.global_vars[760] = 2
	if (attachee.name == 14635 and attachee.leader_get() == OBJ_HANDLE_NULL and attachee.map == 5066):
	# Make the barbarian gnolls patrol around Temple level 1 near Ogre Chief
		rr = game.random_range(1,30)
		if (rr == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 273 )
			attachee.standpoint_set( STANDPOINT_DAY, 273 )
		if (rr == 2):
			attachee.standpoint_set( STANDPOINT_NIGHT, 274 )
			attachee.standpoint_set( STANDPOINT_DAY, 274 )
		if (rr == 3):
			attachee.standpoint_set( STANDPOINT_NIGHT, 275 )
			attachee.standpoint_set( STANDPOINT_DAY, 275 )
		if (rr == 4):
			attachee.standpoint_set( STANDPOINT_NIGHT, 276 )
			attachee.standpoint_set( STANDPOINT_DAY, 276 )
		if (rr == 5):
			attachee.standpoint_set( STANDPOINT_NIGHT, 277 )
			attachee.standpoint_set( STANDPOINT_DAY, 277 )
		rr = 0
	if (attachee.name == 14636 and attachee.leader_get() == OBJ_HANDLE_NULL and attachee.map == 5066):
	# Make the goblin patrol around Temple level 1 near Ogre Chief
		rr = game.random_range(1,30)
		if (rr == 1):
			attachee.standpoint_set( STANDPOINT_NIGHT, 288 )
			attachee.standpoint_set( STANDPOINT_DAY, 288 )
		if (rr == 2):
			attachee.standpoint_set( STANDPOINT_NIGHT, 289 )
			attachee.standpoint_set( STANDPOINT_DAY, 289 )
		if (rr == 3):
			attachee.standpoint_set( STANDPOINT_NIGHT, 290 )
			attachee.standpoint_set( STANDPOINT_DAY, 290 )
		rr = 0
	if (not game.combat_is_active() and attachee.name == 14066 and attachee.map == 5005):
		if (not attachee.has_met(game.party[0])):	
			if (is_better_to_talk(attachee,game.party[0])):
				if (not critter_is_unconscious(game.party[0])):
					if (anyone( game.party[0].group_list(), "has_follower", 8002 )):
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 100 )
					else:
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 1 )
			else: 
				for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
					if (is_safe_to_talk(attachee, obj)):
						if (anyone( game.party[0].group_list(), "has_follower", 8002 )):
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 100 )
						else:
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 1 )
	if (game.global_flags[288] == 1 and game.global_flags[856] == 1 and game.global_flags[857] == 1):
		game.new_sid = 0
		if (is_better_to_talk(attachee,game.party[0])):
			attachee.turn_towards(game.party[0])
			game.party[0].begin_dialog( attachee, 200 )
		else: 
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if ( is_safe_to_talk(attachee,obj)):
					attachee.turn_towards(obj)
					obj.begin_dialog( attachee, 200 )
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_vars[709] >= 2 and attachee.map == 5005):
		attachee.attack(game.leader)
		game.new_sid = 0
		return RUN_DEFAULT
	if (game.global_vars[709] <= 1 and attachee.map == 5005):
		return SKIP_DEFAULT
	if attachee.map == 5091:
		return SKIP_DEFAULT
	return RUN_DEFAULT


def run_off(npc, pc):
	game.global_flags[288] = 1
	location = location_from_axis( 484, 490 )
	npc.runoff(location)
	Timed_Destroy(npc, 5000)
#	game.timevent_add( give_reward, (), 1209600000 )
#	game.timevent_add( give_reward, (), 720000 )
	game.encounter_queue.append(3605)
	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.name == 14079 or obj.name == 14080 or obj.name == 14067 or obj.name == 14078):
			obj.runoff(location)
			Timed_Destroy(obj, 5000)
	return RUN_DEFAULT


def give_reward():
	game.encounter_queue.append(3579)
	return RUN_DEFAULT


def give_item(npc):
	game.global_vars[769] = 0
	item = game.global_vars[768]
	create_item_in_inventory(item,npc)
	game.global_vars[768] = 0
	game.global_vars[776] = game.global_vars[776] + 1
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 20):
			return 1
	return 0
	

def call_leader(npc, pc): 
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 1)
	return 
	

def call_leaderplease(npc, pc):
	leader = game.party[0]
	leader.move(pc.location -2)
	leader.begin_dialog(npc, 100)
	return
	

def call_leadersvp(npc, pc):
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 200)
	return 
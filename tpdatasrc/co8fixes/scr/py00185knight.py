from toee import *
from py00439script_daemon import is_safe_to_talk_rfv
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if triggerer.stat_level_get(stat_alignment) & ALIGNMENT_GOOD == 0:
		good_pc = OBJ_HANDLE_NULL
		for obj in game.party:
			if ( is_safe_to_talk_rfv(attachee, obj, 45, 0, 0) and obj.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0 ):
				good_pc = obj
		if good_pc != OBJ_HANDLE_NULL:
			attachee.turn_towards(triggerer)
			triggerer.begin_dialog(attachee,200) # "I would prefer to speak to the good hearted one"
	elif ( attachee.item_find(3014) != OBJ_HANDLE_NULL ):
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog(attachee,50)
	else:
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog(attachee,1)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	attachee.scripts[19] = 0 # to prevent the "popup after death" bug
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if attachee.scripts[13] == 0:
		attachee.scripts[13] = 185 # assigns san_enter_combat if one doesn't exist
	# This script's execution of searching for a good aligned PC usually fails
	# The reason: it searches for a PC at a distance of 15
	# Very often, your good aligned PC would be standing slightly farther away
	# While another PC would be just at that distance
	# So the script would fail to find the good aligned PC, and find the nearer PC and initiate conversation with that PC
	# Suggested fix: search for a good PC slightly farther away (distance 25)
	# If one is not found, look for any PC at distance 15
	if (not game.combat_is_active()):
		good_pc = OBJ_HANDLE_NULL
		paladin_pc = OBJ_HANDLE_NULL
		good_tank_pc = OBJ_HANDLE_NULL
		good_cleric_pc = OBJ_HANDLE_NULL
		faraway_good_pc = OBJ_HANDLE_NULL
		# near_pc = OBJ_HANDLE_NULL
	
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if ( is_safe_to_talk_rfv(attachee,obj, 25, 0, 1) ):
				# near_pc = obj
				if ( obj.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0 ):
					good_pc = obj
					if ( obj.stat_level_get(stat_level_paladin) > 1 ):
						paladin_pc = obj
					if ( obj.stat_level_get(stat_level_fighter) > 1 ) or ( obj.stat_level_get(stat_level_barbarian) > 1 ) or ( obj.stat_level_get(stat_level_ranger) > 1 ):
						good_tank_pc = obj
					if ( obj.stat_level_get(stat_level_cleric) > 1 ):
						good_cleric_pc = obj
				#if (obj.item_find(3014) != OBJ_HANDLE_NULL ): # Prince Thrommel's Golden Amulet (name ID; shared with halfling sai)
				#	obj.begin_dialog(attachee,50)
				#	game.new_sid = 0
				#	return RUN_DEFAULT
		if (paladin_pc != OBJ_HANDLE_NULL):
			attachee.turn_towards(paladin_pc)
			paladin_pc.begin_dialog(attachee,1)
			game.new_sid = 0
		elif (good_tank_pc != OBJ_HANDLE_NULL):
			attachee.turn_towards(good_tank_pc)
			good_tank_pc.begin_dialog(attachee,1)
			game.new_sid = 0
		elif (good_cleric_pc != OBJ_HANDLE_NULL):
			attachee.turn_towards(good_cleric_pc)
			good_cleric_pc.begin_dialog(attachee,1)
			game.new_sid = 0
		elif (good_pc != OBJ_HANDLE_NULL):
			attachee.turn_towards(good_pc)
			good_pc.begin_dialog(attachee,1)
			game.new_sid = 0
		else:
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if ( is_safe_to_talk_rfv(attachee,obj, 15, 0, 1) ):
					near_pc = obj
					if ( obj.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0 ):
						good_pc = obj
					if (obj.item_find(3014) != OBJ_HANDLE_NULL ): # Prince Thrommel's Golden Amulet (name ID; shared with halfling sai)
						attachee.turn_towards(obj)
						obj.begin_dialog(attachee,50)
						game.new_sid = 0
						return RUN_DEFAULT
				if ( is_safe_to_talk_rfv(attachee,obj, 45, 0, 0) and obj.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0):
					faraway_good_pc = obj
			if (near_pc != OBJ_HANDLE_NULL):
				attachee.turn_towards(near_pc)
				if faraway_good_pc == OBJ_HANDLE_NULL:
					near_pc.begin_dialog(attachee,1)
				else:
					near_pc.begin_dialog(attachee,200)
				game.new_sid = 0
	return RUN_DEFAULT


def distribute_magic_items(npc,pc):
	for obj in pc.group_list():
		obj.money_adj(2000000)
		create_item_in_inventory( 8007, obj )
		create_item_in_inventory( 6082, obj )
	return RUN_DEFAULT


def transfer_scrolls(npc,pc):
	# give out 12 first level wizard scrolls to pc
	create_item_in_inventory( 9288, pc )
	create_item_in_inventory( 9280, pc )
	create_item_in_inventory( 9438, pc )
	create_item_in_inventory( 9431, pc )
	create_item_in_inventory( 9383, pc )
	create_item_in_inventory( 9509, pc )
	create_item_in_inventory( 9467, pc )
	create_item_in_inventory( 9333, pc )
	create_item_in_inventory( 9238, pc )
	create_item_in_inventory( 9229, pc )
	create_item_in_inventory( 9159, pc )
	create_item_in_inventory( 9056, pc )
	return RUN_DEFAULT	


def knight_party(npc,pc):
	pc.reputation_add( 22 )
	for obj in pc.group_list():
		create_item_in_inventory( 6128, obj )
		create_item_in_inventory( 6129, obj )
	return RUN_DEFAULT


def run_off(npc, pc):
	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.leader_get() == OBJ_HANDLE_NULL and not obj in game.leader.group_list()):
			obj.runoff(obj.location-3)
	return


def call_good_pc(npc, pc):
	npc.scripts[19] = 0 # remove heartbeat so it doesn't interfere
	good_pc = OBJ_HANDLE_NULL
	paladin_pc = OBJ_HANDLE_NULL
	good_tank_pc = OBJ_HANDLE_NULL
	good_cleric_pc = OBJ_HANDLE_NULL
	new_talker = OBJ_HANDLE_NULL
	# game.particles( "sp-summon monster I", pc ) # fired ok
	for obj in game.obj_list_vicinity(npc.location,OLC_PC):
		if ( is_safe_to_talk_rfv(npc,obj, 45, 0, 0) and obj.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0):
			#game.particles( 'Orb-Summon-Fire-Elemental', pc )
			good_pc = obj
			if ( obj.stat_level_get(stat_level_paladin) > 1 ):
				paladin_pc = obj
			if ( obj.stat_level_get(stat_level_fighter) > 1 ) or ( obj.stat_level_get(stat_level_barbarian) > 1 ) or ( obj.stat_level_get(stat_level_ranger) > 1 ):
				good_tank_pc = obj
			if ( obj.stat_level_get(stat_level_cleric) > 1 ):
				good_cleric_pc = obj
	if (paladin_pc != OBJ_HANDLE_NULL):
		new_talker = paladin_pc
	elif (good_tank_pc != OBJ_HANDLE_NULL):
		new_talker = good_tank_pc
	elif (good_cleric_pc != OBJ_HANDLE_NULL):
		new_talker = good_cleric_pc
	elif (good_pc != OBJ_HANDLE_NULL):
		new_talker = good_pc
	if new_talker == OBJ_HANDLE_NULL: # failsafe
		new_talker = pc
		new_talker.begin_dialog(npc, 240)
	else:
		new_talker.move(pc.location - 2)
		new_talker.turn_towards(npc)
		npc.turn_towards(new_talker)
		new_talker.begin_dialog(npc, 220)
		return
	return 
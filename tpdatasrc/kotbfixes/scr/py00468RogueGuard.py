from toee import *
from utilities import *

def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	triggerer.begin_dialog( attachee, 100 )
	return SKIP_DEFAULT

def san_first_heartbeat( attachee, triggerer ):
	itemA = attachee.item_find(5004)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.destroy()
		create_item_in_inventory( 5004, attachee)
	itemB = attachee.item_find(5005)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.destroy()
		create_item_in_inventory( 5005, attachee)
	itemC = attachee.item_find(5006)
	if (itemC != OBJ_HANDLE_NULL):
		itemC.destroy()
		create_item_in_inventory( 5006, attachee)
	itemD = attachee.item_find(5007)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.destroy()
		create_item_in_inventory( 5007, attachee)
	itemE = attachee.item_find(5010)
	if (itemE != OBJ_HANDLE_NULL):
		itemE.destroy()
		create_item_in_inventory( 5010, attachee)
	itemF = attachee.item_find(5011)
	if (itemF != OBJ_HANDLE_NULL):
		itemF.destroy()
		create_item_in_inventory( 5011, attachee)
	itemG = attachee.item_find(5013)
	if (itemG != OBJ_HANDLE_NULL):
		itemG.destroy()
		create_item_in_inventory( 5013, attachee)
	return RUN_DEFAULT

def san_dying( attachee, triggerer ):
	game.global_flags[128] = 1
	game.leader.reputation_add( 14 )
	return RUN_DEFAULT

def san_resurrect( attachee, triggerer ):
	game.global_flags[128] = 0
	return RUN_DEFAULT

def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if attachee.has_los(obj):
				if (is_better_to_talk(attachee, game.party[0])):
					if (not critter_is_unconscious(game.party[0])):
						attachee.turn_towards(game.party[0])
						game.leader.begin_dialog( attachee, 1 )
						game.new_sid = 0
				else:
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_safe_to_talk(attachee, obj)):
							attachee.turn_towards(obj)
							obj.begin_dialog(attachee, 1) # fixes invalid dialogue bug
							game.new_sid = 0
	return RUN_DEFAULT

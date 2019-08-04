from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[836] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
#	if (not attachee.has_wielded(4082) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
#	if (not attachee.has_wielded(4082) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[836] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
	if (attachee.map != 5085 and not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			attachee.turn_towards(obj)
			if (is_safe_to_talk(attachee,obj)):
				obj.begin_dialog(attachee,1)
				game.new_sid = 0
	return RUN_DEFAULT


def run_off(npc, pc):
	npc.item_transfer_to_by_proto( pc, 11002)
	npc.runoff(npc.location-3)
	return RUN_DEFAULT
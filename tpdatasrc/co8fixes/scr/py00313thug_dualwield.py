from toee import *
from py00439script_daemon import record_time_stamp, get_v, set_v, npc_set, npc_unset, npc_get, tsc, within_rect_by_corners
from combat_standard_routines import *


def san_enter_combat( attachee, triggerer ):
#	if (not attachee.has_wielded(4082) or not attachee.has_wielded(4112)):
	while (not attachee.has_wielded(4156) or not attachee.has_wielded(4159)):
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
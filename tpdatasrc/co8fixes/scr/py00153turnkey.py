from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if ( game.global_flags[208] == 1) :
		triggerer.begin_dialog( attachee, 110 )
	else:
		triggerer.begin_dialog( attachee, 1 )		
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	#if should_modify_CR( attachee ):
	#	modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	#while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
	#	attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	if ((obj_percent_hp(attachee) < 30) and (game.global_flags[208] == 0)):
		found_pc = OBJ_HANDLE_NULL
		for pc in game.party:
			if pc.type == obj_t_pc:
				found_pc = pc
			else:
				pc.ai_shitlist_remove(attachee)
			attachee.ai_shitlist_remove( pc )
			
		if found_pc != OBJ_HANDLE_NULL:
			found_pc.begin_dialog( attachee, 20 )
			game.new_sid = 0
			return SKIP_DEFAULT
	return RUN_DEFAULT
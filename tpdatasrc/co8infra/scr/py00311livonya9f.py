from __main__ import game;
from toee import *
from combat_standard_routines import *


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
#	attachee.destroy()
#	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):


## THIS IS USED FOR BREAK FREE
	#found_nearby = 0
	#for obj in game.party[0].group_list():
	#	if (obj.distance_to(attachee) <= 9 and obj.stat_level_get(stat_hp_current) >= -9):
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
from toee import *
from combat_standard_routines import *

# Notes on item wielding:
# 4500 - Rapier +2, 4701 - Wakizashi +1 
# 4112 is dagger of venom, useless for AI


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	#if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	#if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	#if (not attachee.has_wielded(4500) or not attachee.has_wielded(4112)):
	if attachee in game.party: # fixes huge lag when charmed (due to doing repeated item_wield_best_all() at high frequency)
		return RUN_DEFAULT
	if (not attachee.has_wielded(4500) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
	return RUN_DEFAULT
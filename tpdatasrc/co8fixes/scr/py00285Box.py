from toee import *
from utilities import *
from combat_standard_routines import *


def san_use( attachee, triggerer ):
	loc = triggerer.location
	npc = game.obj_create( 14687, loc )
	triggerer.begin_dialog( npc, 1 )
	return SKIP_DEFAULT

def san_start_combat( attachee, triggerer ): ## zombies
	randy1 = game.random_range(1,22)
	randy2 = game.random_range(1100,1102)
	if randy1 >= 21:
		attachee.float_mesfile_line( 'mes\\narrative.mes', randy2 )

## ALSO, THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
##		attachee.d20_send_signal(S_BreakFree)

	#Spiritual_Weapon_Begone( attachee )

	return RUN_DEFAULT

def san_insert_item( attachee, triggerer ):
	#print "py000285Box Insert Item. Attachee: " + str(attachee) + " Triggerer: " + str(triggerer)
	# Bonus now doesn't stack in Temple+ - so this workaround is no longer necessary -SA
	#glasses = triggerer.item_find(6031)
	#if glasses != OBJ_HANDLE_NULL and (triggerer.type == obj_t_pc or triggerer.type == obj_t_npc):
	#	triggerer.float_mesfile_line( 'mes\\narrative.mes', 19 )
	#	glasses.destroy()
	return RUN_DEFAULT

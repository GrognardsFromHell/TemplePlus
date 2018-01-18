from toee import *
from utilities import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[697] == 1):
		attachee.object_flag_unset(OF_OFF)
		game.sound( 4137, 1 )
	elif (game.global_vars[697] == 2):
		attachee.object_flag_set(OF_OFF)
	if attachee.name == 8042: # iuz
		attachee.scripts[15] = 365
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	game.global_vars[694] = game.global_vars[694] + 1
	if (game.global_vars[694] == 4):
		game.quests[102].state = qs_completed
		game.party[0].reputation_add(58)
		random_fate()
	return RUN_DEFAULT

def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)

	if attachee.leader_get() != OBJ_HANDLE_NULL: # Don't wanna fuck up charmed enemies
		return RUN_DEFAULT
	
	closest_jones = party_closest( attachee, 1, mode_select = 1, exclude_warded = 1)
	if attachee.distance_to(closest_jones) > 30 and attachee.d20_query_has_spell_condition( sp_Dimensional_Anchor ) == 0:
		attachee.spells_pending_to_memorized()
		attachee.obj_set_int( 324, 110 )
	elif closest_jones == party_closest( attachee, 1, mode_select = 2):
		attachee.obj_set_int( 324, 453 )
	else:
		Spiritual_Weapon_Begone( attachee )


	return RUN_DEFAULT
	


def san_exit_combat( attachee, triggerer ):
	if (attachee.map == 5121):
		attachee.object_flag_set(OF_OFF)
		game.global_vars[697] = 2
	return RUN_DEFAULT


def random_fate():
	pendulum = game.random_range(1,5)
	if (pendulum == 1 or pendulum == 2 or pendulum == 3):
		game.global_vars[508] = 1
	elif (pendulum == 4):
		game.global_vars[508] = 2
	elif (pendulum == 5):
		game.global_vars[508] = 3
	return RUN_DEFAULT
from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 140 )			## meleny in party
	elif (game.global_vars[904] == 32):
		triggerer.begin_dialog( attachee, 360 )			## have attacked 3 or more farm animals with meleny in party
	elif (game.quests[7].state == qs_completed):
		if (game.global_flags[46] == 1):
			triggerer.begin_dialog( attachee, 150 )		## have completed flirting with disaster quest and married meleny
		else:
			triggerer.begin_dialog( attachee, 120 )		## have completed flirting with disaster quest
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[196] = 1
	if (game.global_flags[232] == 0):
		attachee.float_line(12014,triggerer)
		game.global_vars[23] = game.global_vars[23] + 1
		if game.global_vars[23] >= 2:
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
		attachee.float_line(12014,triggerer)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[196] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[904] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[232] = 1
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[232] = 0
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	randy1 = game.random_range(1,12)
	if ((attachee.map == 5035 or attachee.map == 5046 or attachee.map == 5018 or attachee.map == 5086 or attachee.map == 5055) and randy1 >= 6):
		attachee.float_line(12100,triggerer)
	return RUN_DEFAULT


def buttin( attachee, triggerer, line):
	npc = find_npc_near(attachee,8016)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,210)
	return SKIP_DEFAULT


def buttin2( attachee, triggerer, line):
	npc = find_npc_near(attachee,8055)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,330)
	return SKIP_DEFAULT


def buttin3( attachee, triggerer, line):
	npc = find_npc_near(attachee,14037)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,55)
	return SKIP_DEFAULT


def buttin4( attachee, triggerer, line):
	npc = find_npc_near(attachee,8020)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,55)
	return SKIP_DEFAULT


def equip_transfer( attachee, triggerer ):
	itemA = attachee.item_find(6142)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.destroy()
	itemB = attachee.item_find(6143)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.destroy()
	itemC = attachee.item_find(6144)
	if (itemC != OBJ_HANDLE_NULL):
		itemC.destroy()
	itemD = attachee.item_find(6145)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.destroy()
	itemE = attachee.item_find(6146)
	if (itemE != OBJ_HANDLE_NULL):
		itemE.destroy()
	itemF = attachee.item_find(4060)
	if (itemF != OBJ_HANDLE_NULL):
		itemF.destroy()
	itemG = attachee.item_find(4061)
	if (itemG != OBJ_HANDLE_NULL):
		itemG.destroy()
	return RUN_DEFAULT


def switch_to_tarah( attachee, triggerer, line):
	npc = find_npc_near(attachee,8805)
	meleny = find_npc_near(attachee,8015)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc, line)
		npc.turn_towards(meleny)
		meleny.turn_towards(npc)
	return SKIP_DEFAULT
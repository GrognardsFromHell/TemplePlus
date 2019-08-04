from toee import *
from utilities import *
import _include
from co8Util.TimedEvent import *
from combat_standard_routines import *
from py00439script_daemon import get_f, set_f, get_v, set_v, tpsts, record_time_stamp


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 120 )			## thrommel in party
	elif (game.global_vars[907] == 32):
		triggerer.begin_dialog( attachee, 230 )			## have attacked 3 or more farm animals with thrommel in party - probably impossible
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	itemF = attachee.item_find(2201)
	if (itemF != OBJ_HANDLE_NULL):
		itemF.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[150] = 1
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[150] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[907] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	itemD = attachee.item_find(2201)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ((attachee.map == 5062) or (attachee.map == 5111) or (attachee.map == 5112) or (attachee.map == 5001) or (attachee.map == 5121)):
	# 5062 - Temple 
	# 5111 - Temple Tower
	# 5112 - Ramshackle Farm (secret entrance to Temple Tower / level 3
	# 5001 - Hommlet
	# 5121 - Verbobonc
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 130 )
	return RUN_DEFAULT


def san_true_seeing( attachee, triggerer ):
	if (attachee.can_see(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_dispel_magic ):
		triggerer.begin_dialog( attachee, 1 )
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	game.global_flags[151] = 1
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def check_follower_thrommel_comments( attachee, triggerer ):
	npc = find_npc_near( attachee, 8014)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( npc, 490 )
	else:
		npc = find_npc_near( attachee, 8000)
		if (npc != OBJ_HANDLE_NULL):
			triggerer.begin_dialog( npc, 550 )
		else:
			triggerer.begin_dialog( attachee, 10 )
	return RUN_DEFAULT


def schedule_reward( attachee, triggerer ):
	game.global_flags[152] = 1
	attachee.object_flag_set(OF_OFF)
	game.timevent_add( give_reward, (), 1209600000 ) #1209600000ms is 2 weeks
	record_time_stamp('s_thrommel_reward')
	return RUN_DEFAULT


def give_reward():
	game.encounter_queue.append(3001)
	set_f('s_thrommel_reward_scheduled')
	return RUN_DEFAULT


def equip_transfer( attachee, triggerer ):
	itemA = attachee.item_find(3014)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.item_flag_unset(OIF_NO_TRANSFER)
		attachee.item_transfer_to(triggerer,3014)
	return RUN_DEFAULT


def equip_transfer2( attachee, triggerer ):
	if anyone( game.leader.group_list(), "has_item", 2201):
		party_transfer_to( attachee, 2201 )
	return RUN_DEFAULT
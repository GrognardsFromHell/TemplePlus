from toee import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.map == 5172):
		if (game.global_vars[945] == 1 or game.global_vars[945] == 2 or game.global_vars[945] == 3):
			triggerer.begin_dialog( attachee, 660 )
		else:
			triggerer.begin_dialog( attachee, 670 )	
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[989] == 1):
		attachee.object_flag_set(OF_OFF)
	elif (attachee.map == 5160):
		if (game.global_flags[982] == 1):
			attachee.object_flag_unset(OF_OFF)
			if (game.global_vars[944] >= 1):
				attachee.object_flag_set(OF_OFF)
	elif (attachee.map == 5172):
		if (game.global_vars[945] == 1 or game.global_vars[945] == 2 or game.global_vars[945] == 3):
			attachee.object_flag_unset(OF_OFF)
		elif (game.global_flags[943] == 1 or game.global_vars[945] == 28 or game.global_vars[945] == 29 or game.global_vars[945] == 30):
			attachee.object_flag_set(OF_OFF)
	else:
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[989] = 1
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[989] = 0
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
#	if (not attachee.has_wielded(4082) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4700) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
#	if (not attachee.has_wielded(4082) or not attachee.has_wielded(4112)):
	if (not attachee.has_wielded(4700) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
#		game.new_sid = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not attachee.has_wielded(4700) or not attachee.has_wielded(4701)):
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def run_off(npc, pc):
	npc.item_transfer_to_by_proto( pc, 7003 )
	npc.runoff(npc.location-3)
	return RUN_DEFAULT


def party_spot_check():
	highest_spot = -999
	for pc in game.party:
		if pc.skill_level_get(skill_spot) > highest_spot:
			highest_spot = pc.skill_level_get(skill_spot) 
	return highest_spot


def wilfrick_countdown( attachee, triggerer ):
	game.timevent_add( stop_watch, (), 172800000 )	## 2 days
	return RUN_DEFAULT


def stop_watch():
	game.global_vars[704] = 2
	return RUN_DEFAULT


def darlia_release( attachee, triggerer ):
	game.timevent_add( cut_loose, (), 345600000 )	## 4 days
	return RUN_DEFAULT


def cut_loose():
	game.global_flags[943] = 1
	return RUN_DEFAULT


def schedule_sb_retaliation_for_snitch( attachee, triggerer ):
	game.global_vars[945] = 4
	game.timevent_add( sb_retaliation_for_snitch, (), 864000000 ) #864000000ms is 10 days
	record_time_stamp('s_sb_retaliation_for_snitch')
	return RUN_DEFAULT


def schedule_sb_retaliation_for_narc( attachee, triggerer ):
	game.global_vars[945] = 5
	game.timevent_add( sb_retaliation_for_narc, (), 518400000 ) #518400000ms is 6 days
	record_time_stamp('s_sb_retaliation_for_narc')
	return RUN_DEFAULT


def schedule_sb_retaliation_for_whistleblower( attachee, triggerer ):
	game.global_vars[945] = 6
	game.timevent_add( sb_retaliation_for_whistleblower, (), 1209600000 ) #1209600000ms is 14 days
	record_time_stamp('s_sb_retaliation_for_whistleblower')
	return RUN_DEFAULT


def sb_retaliation_for_snitch():
	game.encounter_queue.append(3435)
	set_f('s_sb_retaliation_for_snitch_scheduled')
	return RUN_DEFAULT


def sb_retaliation_for_narc():
	game.encounter_queue.append(3435)
	set_f('s_sb_retaliation_for_narc_scheduled')
	return RUN_DEFAULT


def sb_retaliation_for_whistleblower():
	game.encounter_queue.append(3435)
	set_f('s_sb_retaliation_for_whistleblower_scheduled')
	return RUN_DEFAULT
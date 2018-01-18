from toee import *
from scripts import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if (game.global_vars[914] == 32 and attachee.map != 5149):
			triggerer.begin_dialog( attachee, 340 )		## have killed 3 or more farm animals with holly in party and not in verbo watch post main floor
		elif (game.global_vars[976] == 2):
			attachee.turn_towards(triggerer)
			triggerer.begin_dialog( attachee, 1 )		## captain absalom offered to send holly with you to investigate drow and you accepted
		elif (is_daytime() == 1):
			triggerer.begin_dialog( attachee, 10 )		## generic daytime
		else:
			triggerer.begin_dialog( attachee, 20 )		## generic nighttime
	else:
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog( attachee, 170 )			## holly in party
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if ( ( attachee.map == 5149 ) and ( game.global_vars[976] == 4 ) ):
		attachee.object_flag_unset(OF_OFF)
		game.global_vars[976] = 5
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[962] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[962] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if ( (game.party[0].reputation_has(34) == 1 or game.party[0].reputation_has(35) == 1 or game.party[0].reputation_has(42) == 1 or game.party[0].reputation_has(44) == 1 or game.party[0].reputation_has(35) == 1 or game.party[0].reputation_has(43) == 1 or game.party[0].reputation_has(46) == 1 or (game.global_vars[993] == 5 and game.global_flags[870] == 0)) ):
		if ( (game.global_vars[969] == 0) and (game.global_flags[955] == 0) ):
			if (not game.combat_is_active()):
				for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
					if (is_better_to_talk(attachee,obj)):
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 320 )
						game.global_vars[969] = 1
	elif (not game.combat_is_active()):
		if (game.global_vars[914] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if (game.party[0].reputation_has(34) == 1) or (game.party[0].reputation_has(35) == 1):
			return RUN_DEFAULT
		elif (game.global_flags[992] == 0):
			return SKIP_DEFAULT
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	cloak = attachee.item_find( 6269 )
	cloak.item_flag_set(OIF_NO_TRANSFER)
	armor = attachee.item_find( 6103 )
	armor.item_flag_set(OIF_NO_TRANSFER)
	boots = attachee.item_find( 6040 )
	boots.item_flag_set(OIF_NO_TRANSFER)
	gloves = attachee.item_find( 6041 )
	gloves.item_flag_set(OIF_NO_TRANSFER)
	helm = attachee.item_find( 6335 )
	helm.item_flag_set(OIF_NO_TRANSFER)
	shield = attachee.item_find( 6499 )
	shield.item_flag_set(OIF_NO_TRANSFER)
	sword = attachee.item_find( 4030 )
	sword.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_vars[976] = 4
	cloak = attachee.item_find( 6269 )
	cloak.item_flag_unset(OIF_NO_TRANSFER)
	armor = attachee.item_find( 6103 )
	armor.item_flag_unset(OIF_NO_TRANSFER)
	boots = attachee.item_find( 6040 )
	boots.item_flag_unset(OIF_NO_TRANSFER)
	gloves = attachee.item_find( 6041 )
	gloves.item_flag_unset(OIF_NO_TRANSFER)
	helm = attachee.item_find( 6335 )
	helm.item_flag_unset(OIF_NO_TRANSFER)
	shield = attachee.item_find( 6499 )
	shield.item_flag_unset(OIF_NO_TRANSFER)
	sword = attachee.item_find( 4030 )
	sword.item_flag_unset(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		if ( attachee.map == 5121 ):
			if ( game.global_vars[999] >= 15 ):
				attachee.turn_towards(triggerer)
				game.leader.begin_dialog( attachee, 30 )
			elif ( game.quests[66].state == qs_accepted ) or ( game.quests[67].state == qs_accepted ) or ( game.quests[77].state == qs_accepted ):
				attachee.turn_towards(triggerer)
				game.leader.begin_dialog( attachee, 200 )
		elif ( (attachee.map == 5158) and (game.global_flags[959] == 0) ):
			game.global_flags[959] = 1
			game.leader.begin_dialog( attachee, 210 )
		elif ( attachee.map == 5007 ) or ( attachee.map == 5060 ) or ( attachee.map == 5151 ):
			if ( (game.global_vars[968] == 1) and (is_daytime() == 0) ):
				game.leader.begin_dialog( attachee, 220 )
		elif ( attachee.map == 5008 ) or ( attachee.map == 5061 ) or ( attachee.map == 5152 ):
			if ( (game.global_vars[968] == 9) and (is_daytime() == 0) ):
				attachee.turn_towards(triggerer)
				game.leader.begin_dialog( attachee, 260 )
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def get_holly_drunk( attachee, triggerer ):
	game.global_vars[968] = game.global_vars[968] + 1
	ale = triggerer.item_find_by_proto(8004)
	ale.destroy()
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 35):
			return 1
	return 0
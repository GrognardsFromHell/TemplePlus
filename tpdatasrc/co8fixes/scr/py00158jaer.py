from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 100 )
	elif (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 90 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				game.new_sid = 0
				obj.begin_dialog(attachee,1)
				return RUN_DEFAULT
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ((attachee.area == 1) or (attachee.area == 3) or (attachee.area == 14)):
		obj = attachee.leader_get()
		if (obj != OBJ_HANDLE_NULL):
			obj.begin_dialog(attachee, 140)
	return RUN_DEFAULT


def transfer_fire_balls( attachee, triggerer ):
	while (attachee.item_find(2207)  != OBJ_HANDLE_NULL):
		attachee.item_transfer_to(triggerer,2207)
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT
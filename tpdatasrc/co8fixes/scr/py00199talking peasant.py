from toee import *
from combat_standard_routines import *
from utilities import *

def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog(attachee,1)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (critter_is_unconscious(obj) == 0):
			attachee.turn_towards(obj)
			obj.begin_dialog(attachee,1)
			game.new_sid = 0
			return RUN_DEFAULT
	return RUN_DEFAULT

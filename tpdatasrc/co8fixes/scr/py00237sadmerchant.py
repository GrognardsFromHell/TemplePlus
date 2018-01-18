from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 30 )		
	return SKIP_DEFAULT

def san_heartbeat(attachee, triggerer):
	pass

def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[354] == 0):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT



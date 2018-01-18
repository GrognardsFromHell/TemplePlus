from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.obj_create(14324, attachee.location)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):	
	attachee.turn_towards(triggerer)
	attachee.float_line(1,triggerer)
	attachee.critter_flag_set( OCF_MUTE )
	return RUN_DEFAULT


def san_exit_combat( attachee, triggerer ):
	if (attachee.stat_level_get(stat_subdual_damage) >= attachee.stat_level_get(stat_hp_current)):
		game.obj_create(14324, attachee.location)
	return RUN_DEFAULT
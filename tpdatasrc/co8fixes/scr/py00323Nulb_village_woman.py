from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_vars[958] == 1):
		triggerer.begin_dialog( attachee, 1000 )
	elif (game.global_vars[958] == 2):
		triggerer.begin_dialog( attachee, 2000 )
	elif (game.global_vars[958] == 10):
		triggerer.begin_dialog( attachee, 5000 )
	elif (game.global_vars[958] == 3):
		triggerer.begin_dialog( attachee, 3000 )
	else:
		return SKIP_DEFAULT
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[958] == 2):
		attachee.object_flag_unset(OF_OFF)
		game.particles("ef-Node Portal", attachee)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[958] == 1):
		attachee.object_flag_unset(OF_OFF)
		if (not game.combat_is_active()):
			if (is_better_to_talk(attachee, game.party[0])):
				game.party[0].begin_dialog( attachee, 1000 )
	elif (game.global_vars[958] == 4):
		if (not game.combat_is_active()):
			if (is_better_to_talk(attachee, game.party[0])):
				game.party[0].begin_dialog( attachee, 4000 )
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 20):
			return 1
	return 0


def get_sick( attachee, triggerer ):
	for obj in game.obj_list_vicinity( triggerer.location, OLC_PC ):
		obj.condition_add_with_args("Poisoned",15,0)
		obj.condition_add_with_args("Poisoned",32,0)
		obj.condition_add_with_args("Poisoned",18,0)
		obj.condition_add_with_args("Poisoned",29,0)
	return RUN_DEFAULT
	

def all_die( attachee, triggerer ):
	game.global_vars[958] = 7
	for pc in game.party:
		pc.critter_kill_by_effect()
		game.particles( "sp-Poison", pc )
	return 1


def stop_watch( attachee, triggerer ):
	game.timevent_add( all_dead, (), 36000000 )	# 10 hours
	game.global_vars[958] = 6
	return RUN_DEFAULT


def all_dead():
	if (game.global_vars[958] == 6):
		game.global_vars[958] = 7
		for pc in game.party:
			pc.critter_kill_by_effect()
			game.particles( "sp-Poison", pc )
	return 1
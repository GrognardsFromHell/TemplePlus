from utilities import *
from toee import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5005):
		if (game.quests[95].state == qs_mentioned and game.global_vars[752] >= 9):
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT

	
def san_heartbeat(attachee, triggerer):
	pass

def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_vars[752] = game.global_vars[752] + 1

	# Record time when you killed a moathouse Gnoll
	if game.global_vars[405] == 0:
		game.global_vars[405] = game.time.time_game_in_seconds(game.time)
	return RUN_DEFAULT
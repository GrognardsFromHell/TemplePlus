from toee import *
from combat_standard_routines import *
from utilities import *

def san_heartbeat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (critter_is_unconscious(obj) == 0):
			#attachee.turn_towards(obj)
			if not game.tutorial_is_active():
				game.tutorial_toggle()
			game.tutorial_show_topic( TAG_TUT_ROOM1_OVERVIEW )
			game.new_sid = 0
			return RUN_DEFAULT
	return RUN_DEFAULT
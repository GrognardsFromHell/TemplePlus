from toee import *
from combat_standard_routines import *
from utilities import *

def san_heartbeat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (critter_is_unconscious(obj) == 0):
			if game.global_flags[0] == 0:
				if not game.tutorial_is_active():
					game.tutorial_toggle()
				game.tutorial_show_topic( TAG_TUT_DIALOGUE )
				game.global_flags[0] = 1
				game.global_flags[10] = 1
			elif game.global_flags[10] == 1:
				#if not game.tutorial_is_active():
				obj.begin_dialog( attachee, 1 )
				game.new_sid = 0
				game.global_flags[10] = 0
	return RUN_DEFAULT
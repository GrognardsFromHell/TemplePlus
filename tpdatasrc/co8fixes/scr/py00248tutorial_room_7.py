from toee import *
from utilities import *


def correct_zombie_factions():
	for obj in game.obj_list_vicinity(location_from_axis(464, 487) ,OLC_NPC):
		if obj in game.party:
			continue
		if obj.faction_has(7) == 0:
			obj.faction_add(7)
	return

def san_heartbeat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		if (critter_is_unconscious(obj) == 0):
			if attachee.distance_to( obj ) < 30:
				if not game.tutorial_is_active():
					game.tutorial_toggle()
				game.tutorial_show_topic( TAG_TUT_ROOM7_OVERVIEW )
				game.global_flags[6] = 1
				game.new_sid = 0
				correct_zombie_factions()
				return RUN_DEFAULT
	return RUN_DEFAULT
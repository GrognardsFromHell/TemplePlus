from toee import *
from toee import anyone
from combat_standard_routines import *
from utilities import *


def san_dialog( attachee, triggerer ):
	if (anyone(triggerer.group_list(),"has_wielded",3005)):
		if (attachee.has_met(triggerer)):
			triggerer.begin_dialog( attachee, 100 )
		else:
			triggerer.begin_dialog( attachee, 1 )
	else:
		attachee.attack(triggerer)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[59] = 1
	game.global_vars[756] = game.global_vars[756] + 1

	# Record time when you killed Lubash
	if game.global_vars[406] == 0:
		game.global_vars[406] = game.time.time_game_in_seconds(game.time)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[59] = 0
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (anyone(triggerer.group_list(),"has_wielded",3005)):
		return SKIP_DEFAULT
	else:
		return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	cloak = anyone(game.party[0].group_list(), "has_wielded", 3005)
	if (not game.combat_is_active()):
		if (not attachee.has_met(game.party[0])):
			if (is_better_to_talk(attachee, game.party[0])):
				if (cloak and not critter_is_unconscious(game.party[0])):
					attachee.turn_towards(game.party[0])
					game.party[0].begin_dialog(attachee,1)
					game.new_sid = 0
			else:
				for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
					if (is_safe_to_talk(attachee, obj)):
						if cloak:
							attachee.turn_towards(obj)
							obj.begin_dialog(attachee,1)
	return RUN_DEFAULT


def is_better_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 20):
			return 1
	return 0


def call_leader(npc, pc):
	leader = game.party[0]
	leader.move(pc.location - 2)
	leader.begin_dialog(npc, 1)
	return 
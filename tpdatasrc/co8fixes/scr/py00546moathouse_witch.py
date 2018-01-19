from utilities import *
from toee import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5005):
		if (game.quests[95].state == qs_mentioned and game.global_vars[756] >= 7):
			attachee.object_flag_unset(OF_OFF)
			attachee.cast_spell(spell_stoneskin, attachee)
			attachee.spells_pending_to_memorized()
			game.global_vars[566] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	game.global_flags[249] = 1
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if attachee.name == 14958:	## Nightwalker
		dummy = 1
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.map == 5005):
		if (not game.combat_is_active()):
			for dude in game.party:
				if (attachee.distance_to(dude) <= 100):
					game.global_vars[566] = game.global_vars[566] + 1
					if (attachee.leader_get() == OBJ_HANDLE_NULL):
						if (game.global_vars[566] == 4):
							attachee.cast_spell(spell_mislead, attachee)
							attachee.spells_pending_to_memorized()
						if (game.global_vars[566] >= 400):
							game.global_vars[566] = 0
					return RUN_DEFAULT
	return RUN_DEFAULT
from toee import *
from utilities import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5005):
		if (game.quests[95].state == qs_mentioned and game.global_vars[755] >= 9):
			attachee.object_flag_unset(OF_OFF)
			game.global_vars[565] = 0
			attachee.scripts[19] = 551
	attachee.d20_send_signal(S_SetPowerAttack, 5)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for dude in game.party:
			if (attachee.distance_to(dude) <= 100):
				game.global_vars[565] = game.global_vars[565] + 1
				if (attachee.leader_get() == OBJ_HANDLE_NULL):
					if (game.global_vars[565] == 4):
						attachee.cast_spell(spell_see_invisibility, attachee)
						attachee.spells_pending_to_memorized()
					if (game.global_vars[565] >= 400):
						game.global_vars[565] = 0
				break
#				game.sound( 4171, 1 )
	return RUN_DEFAULT


## san_heartbeat currently off as nightwalker won't buff see_invisibility if it is domain_special, and it will last 1 second if it is class_wizard. Only other buff spell given by Ted is haste - nightwalker is immune.


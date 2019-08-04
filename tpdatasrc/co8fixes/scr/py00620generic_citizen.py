from utilities import *
from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	attachee.float_line( 1000, triggerer )
	return SKIP_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	name_exceptions = [14012, # Renton (militia captain)
		14023, # Percy
		14044, # Woodcutter
		20001, # Jaroo
		14321 # CE vignette peasant
		]
	if not (attachee.name in name_exceptions):
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
			attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_exit_combat( attachee, triggerer ):
	attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	game.particles( "mon-Chicken-white-hit", attachee )
	attachee.float_mesfile_line( 'mes\\float.mes', 5 )
	return SKIP_DEFAULT


def san_will_kos( attachee, triggerer ):
	return SKIP_DEFAULT
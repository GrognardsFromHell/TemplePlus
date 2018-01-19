from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if ( attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 200 )
	else:
		triggerer.begin_dialog( attachee, 100 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.quests[95].state == qs_mentioned and game.global_vars[764] >= 8):
		attachee.object_flag_unset( OF_OFF )
		game.new_sid = 0
	return RUN_DEFAULT


def behave(attachee, triggerer):
	attachee.npc_flag_unset(ONF_WAYPOINTS_DAY)
	attachee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if obj.name == 14686:
			obj.npc_flag_unset(ONF_WAYPOINTS_DAY)
			obj.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
	return


def bling( attachee, triggerer ):
	game.sound( 4048, 1 )
	for pc in game.party:
		game.particles( "sp-Neutralize Poison", pc )
	return 1
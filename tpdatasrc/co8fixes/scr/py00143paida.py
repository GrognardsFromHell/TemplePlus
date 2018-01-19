from toee import *
from utilities import *
from Co8 import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 110 )			## paida in party
	elif (game.global_vars[902] == 32 and attachee.map != 5044):
		triggerer.begin_dialog( attachee, 240 )			## have attacked 3 or more farm animals with paida in party and paida not at home
	elif (game.global_flags[148] == 1) and (game.global_flags[932] == 0):
		triggerer.begin_dialog( attachee, 70 )			## paida dispelled and has not talked to you yet since
	elif (game.global_flags[148] == 0):
		if (game.global_flags[325] == 1):
			triggerer.begin_dialog( attachee, 230 )		## paida not dispelled and hedrack killed in front of her
		else:
			triggerer.begin_dialog( attachee, 1 )		## paida not dispelled
	elif (game.global_flags[38] == 1):
		triggerer.begin_dialog( attachee, 170 )			## paida has been returned to valden
	else:
		triggerer.begin_dialog( attachee, 160 )			## none of the above - will fire angry dialog like you left her in temple
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if ( ( game.global_flags[149] == 1 ) or ( game.global_flags[38] == 1 ) ):
		if ( attachee.map == 5044 ):
			if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
				attachee.object_flag_set(OF_OFF)
			else:
				attachee.object_flag_unset(OF_OFF)
		if ( attachee.map == 5080 ):
			attachee.object_flag_set(OF_OFF)
	elif ( attachee.map == 5044 and attachee.leader_get() == OBJ_HANDLE_NULL):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[153] = 1
	if (game.global_flags[238] == 0):
		attachee.float_line(12014,triggerer)
		game.global_vars[23] = game.global_vars[23] + 1
		if game.global_vars[23] >= 2:
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
		attachee.float_line(12014,triggerer)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[153] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		StopCombat(attachee, 1)
		if (game.quests[20].state == qs_completed):
			for pc in game.party:
				if pc.has_follower(8001):
					pc.follower_remove( attachee )
					game.new_sid = 0
		elif (game.global_vars[902] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
					if (attachee.map == 5001):
						game.quests[20].state = qs_completed
						game.global_flags[38] = 1
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[238] = 1
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[238] = 0
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_dispel_magic or spell.spell == spell_break_enchantment or spell.spell == spell_dispel_evil ):
		game.global_flags[148] = 1
		triggerer.begin_dialog( attachee, 70 )
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	#attachee.standpoint_set( STANDPOINT_NIGHT, 257 )
	#attachee.standpoint_set( STANDPOINT_DAY, 257 )
	attachee.runoff(attachee.location-6)
	return RUN_DEFAULT


def LookHedrack( attachee, triggerer, line):
	npc = find_npc_near(attachee,8032)
	if (npc != OBJ_HANDLE_NULL) and ( game.global_flags[146] == 0 ):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,10)
	return SKIP_DEFAULT


def get_rep( attachee, triggerer ):
	if triggerer.reputation_has( 7 ) == 0:
		triggerer.reputation_add( 7 )
	game.global_vars[25] = game.global_vars[25] + 1
	if ( game.global_vars[25] >= 3 and triggerer.reputation_has( 8 ) == 0):
		triggerer.reputation_add( 8 )
	return RUN_DEFAULT
from __main__ import game;
from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_flags[840] == 1 and attachee.name == 14614):
		triggerer.begin_dialog( attachee, 500 )
	elif (game.global_flags[840] == 1 and attachee.name == 14617):
		triggerer.begin_dialog( attachee, 600 )
	elif (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	elif (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 200 )
	elif (game.global_flags[837] == 1):
		triggerer.begin_dialog( attachee, 100 )

	else:
		rr = game.random_range(1,13)
		rr = rr + 79
		attachee.float_line(rr,triggerer)
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	game.global_vars[758] = 0
	game.global_vars[726] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
		game.global_flags[37] = 1
		if ( game.story_state <= 1 ):
			game.story_state = 2
		return RUN_DEFAULT
	game.global_flags[37] = 1
	if ( game.story_state <= 1 ):
		game.story_state = 2
	for pc in game.party:
		if ( pc.reputation_has( 18 ) == 1):
			pc.reputation_remove( 18 )
	game.party[0].reputation_add( 15 )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (attachee.name == 14618):
		attachee.destroy()
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	if (attachee.name == 14618):
		attachee.destroy()
	if (game.global_flags[837] == 0):
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[37] = 0
	for pc in game.party:
		if ( pc.reputation_has( 15 ) == 1):
			pc.reputation_remove( 15 )
	for pc in game.party:
		if ( pc.reputation_has( 18 ) == 0):
			pc.reputation_add( 18 )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):	
	if (attachee.name == 14618):
		attachee.destroy()
	if (game.global_flags[837] == 0):
		if (game.global_vars[758] == 0):
			game.particles( "sp-Hold Person", attachee )
		if (game.global_vars[758] == 1):
			game.particles( "sp-Bestow Curse", attachee )
		game.global_vars[758] = game.global_vars[758] + 1
		if (game.global_vars[758] >= 13):
			game.global_vars[758] = 1
	if (game.global_vars[726] == 0 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active() and attachee.map == 5065):
		attachee.cast_spell(spell_endurance, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[726] == 4 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active() and attachee.map == 5065):
		attachee.cast_spell(spell_magic_circle_against_evil, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[726] == 8 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active() and attachee.map == 5065):
		attachee.cast_spell(spell_owls_wisdom, attachee)
		attachee.spells_pending_to_memorized()
	game.global_vars[726] = game.global_vars[726] + 1
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_flags[840] == 1 and attachee.map == 5065):
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_dispel_magic and attachee.map == 5014 and game.global_flags[837] == 0):
		game.global_flags[837] = 1
		loc = attachee.location
		cur = attachee.stat_level_get( stat_hp_current )
		max = attachee.stat_level_get( stat_hp_max )
		new = max - cur
		damage_dice = dice_new( '1d1' )
 		damage_dice.number = new
		attachee.destroy()
		npc = game.obj_create( 14614, location_from_axis (490L, 483L))
#		while (i < total):
		npc.damage( OBJ_HANDLE_NULL, 0, damage_dice)
		if (game.global_flags[838] == 1):
			game.party[0].begin_dialog( npc, 100 )
		else:
			game.party[0].begin_dialog( npc, 120 )
	if (attachee.map == 5014 and game.global_flags[837] == 0):
		triggerer.float_mesfile_line( 'mes\\spell.mes', 30000 )
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	if (game.global_flags[806] == 0):
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
			attachee.reaction_set( pc, 50 )
		for npc in game.obj_list_vicinity(attachee.location,OLC_NPC):
			for pc in game.party:
				npc.ai_shitlist_remove( pc )
				npc.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if (attachee.map == 5065):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 400 )
	if (attachee.map != 5014 and attachee.map != 5015 and attachee.map != 5016 and attachee.map != 5017 and attachee.map != 5018 and attachee.map != 5019 and attachee.map != 5065):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.begin_dialog( attachee, 300 )
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def create_store( attachee, triggerer ):
	loc = attachee.location
	target = game.obj_create( 14618, loc)
#	triggerer.barter(target)
	triggerer.begin_dialog( target, 700 )
	return SKIP_DEFAULT

def remove_combat( attachee, triggerer ):
	for pc in game.party:
		if (pc.type == obj_t_pc):
			attachee.ai_shitlist_remove( pc )
	return RUN_DEFAULT
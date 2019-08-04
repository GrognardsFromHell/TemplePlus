from toee import *
from Co8 import *
from combat_standard_routines import *
from utilities import *

def san_dialog( attachee, triggerer ):
	if ( game.global_flags[144] == 1 ):
		if (not attachee.has_met(triggerer)):
			triggerer.begin_dialog( attachee, 10 )
		else:
			triggerer.begin_dialog( attachee, 290 )
	elif ( game.quests[58].state >= qs_accepted ):
			triggerer.begin_dialog( attachee, 480 )
	elif ( attachee.has_met(triggerer) ):
			triggerer.begin_dialog( attachee, 490 )
	else:
			triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	print "Hedrack First Heartbeat"
	if (game.global_flags[372] == 1):
		attachee.object_flag_set(OF_OFF)
	else:
		if (game.global_vars[754] == 1):
			return RUN_DEFAULT
		if (not game.combat_is_active()):
			game.global_vars[719] = 0
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (is_safe_to_talk(attachee,obj)):
					if ( game.global_vars[691] == 3 ):
						obj.turn_towards(attachee)		## added by Livonya
						attachee.turn_towards(obj)		## added by Livonya
						obj.begin_dialog( attachee, 40 )
					elif ( game.global_vars[691] == 2 ):
						obj.turn_towards(attachee)		## added by Livonya
						attachee.turn_towards(obj)		## added by Livonya
						obj.begin_dialog( attachee, 30 )
					elif ( game.global_vars[691] == 1 ):
						obj.turn_towards(attachee)		## added by Livonya
						attachee.turn_towards(obj)		## added by Livonya
						obj.begin_dialog( attachee, 20 )
					else:
						obj.turn_towards(attachee)		## added by Livonya
						attachee.turn_towards(obj)		## added by Livonya
						obj.begin_dialog(attachee,1)
					game.global_vars[754] = 1
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[146] = 1
	return RUN_DEFAULT


def san_end_combat( attachee, triggerer ):
	npc = find_npc_near(attachee,8001)
	if (npc != OBJ_HANDLE_NULL):
		game.global_flags[325] = 1
	npc = find_npc_near(attachee,8059)
	if (npc != OBJ_HANDLE_NULL):
		game.global_flags[325] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	print "Hedrack san_start_combat"
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		game.global_vars[744] = game.global_vars[744] + 1
		if (game.global_vars[744] == 3 and game.global_flags[823] == 0 and game.global_flags[147] == 0 and game.global_flags[990] == 0):
			shocky_backup = game.obj_create( 14233, attachee.location-8 )
			shocky_backup.turn_towards(attachee)
			game.sound( 4035, 1 )
			game.particles( "sp-Teleport", shocky_backup )
			racky = attachee.get_initiative()
			shocky_backup.add_to_initiative()
			shocky_backup.set_initiative( racky )
			game.update_combat_ui()
			for obj in game.obj_list_vicinity(shocky_backup.location,OLC_PC):
				shocky_backup.attack(obj)
			game.global_flags[823] = 1
		if (obj_percent_hp(attachee) <= 50):
			if (game.global_flags[377] == 0):
				StopCombat(attachee, 0)
				delegatePc = GetDelegatePc(attachee)
				print "Hedrack: Stopping combat. Delegate PC selected for dialog is " + str(delegatePc)
				for pc in game.party:
					attachee.ai_shitlist_remove( pc )
				if (delegatePc != OBJ_HANDLE_NULL):
					delegatePc.turn_towards(attachee)
					attachee.turn_towards(delegatePc)
					delegatePc.begin_dialog( attachee, 190 )
					game.global_flags[377] = 1
					return SKIP_DEFAULT
			else:
				if (game.global_vars[781] <= 5):
					attachee.obj_set_int(obj_f_critter_strategy, 469)
					game.global_vars[781] = game.global_vars[781] + 1
				elif (game.global_vars[780] <= 8):
					if (game.global_vars[782] >= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 470)
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 468)
					game.global_vars[780] = game.global_vars[780] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 472)
		elif (obj_percent_hp(attachee) >= 51 and obj_percent_hp(attachee) <= 75):
			if (game.global_flags[377] == 0):
				if (game.global_vars[781] <= 5):
					attachee.obj_set_int(obj_f_critter_strategy, 469)
					game.global_vars[781] = game.global_vars[781] + 1
				elif (game.global_vars[780] <= 8):
					if (game.global_vars[782] >= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 470)
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 468)
					game.global_vars[780] = game.global_vars[780] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 472)
			else:
				if (game.global_vars[781] <= 5):
					attachee.obj_set_int(obj_f_critter_strategy, 469)
					game.global_vars[781] = game.global_vars[781] + 1
				elif (game.global_vars[780] <= 8):
					if (game.global_vars[782] >= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 470)
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 468)
					game.global_vars[780] = game.global_vars[780] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 472)
		else:
			if (game.global_flags[377] == 0):
				if (game.global_vars[780] <= 8):
					if (game.global_vars[782] >= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 470)
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 468)
					game.global_vars[780] = game.global_vars[780] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 472)
			else:
				if (game.global_vars[780] <= 8):
					if (game.global_vars[782] >= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 470)
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 468)
					game.global_vars[780] = game.global_vars[780] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 472)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[146] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.combat_is_active()):
		return RUN_DEFAULT
	else:
		print "Hedrack Heartbeat"
		closest_jones = party_closest( attachee)
		if (attachee.distance_to(closest_jones) <= 100):
			game.global_vars[719] = game.global_vars[719] + 1
			if (attachee.leader_get() == OBJ_HANDLE_NULL):
				if (game.global_vars[719] == 4):
					attachee.cast_spell(spell_freedom_of_movement, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[719] == 8):
					attachee.cast_spell(spell_owls_wisdom, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[719] == 12):
					attachee.cast_spell(spell_shield_of_faith, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[719] == 16):
					attachee.cast_spell(spell_protection_from_good, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[719] == 20):
					attachee.cast_spell(spell_protection_from_law, attachee)
					attachee.spells_pending_to_memorized()
			if (game.global_vars[719] >= 400):
				game.global_vars[719] = 0
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_28_and_under(attachee, obj) and game.global_flags[812] == 0):
				if ( game.quests[58].state != qs_unknown ):
					game.global_flags[812] = 1
					return SKIP_DEFAULT
				elif ( game.global_vars[691] == 3 ):
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 40 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 40 )
				elif ( game.global_vars[691] == 2 ):
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 30 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 30 )
				elif ( game.global_vars[691] == 1 ):
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 20 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 20 )
				elif ( game.global_flags[144] == 1 ):
					if (not attachee.has_met(obj)):
						if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
							game.party[0].turn_towards(attachee)
							attachee.turn_towards(game.party[0])
							game.party[0].begin_dialog( attachee, 10 )
						else:
							obj.turn_towards(attachee)
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 10 )
					else:
						if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
							game.party[0].turn_towards(attachee)
							attachee.turn_towards(game.party[0])
							game.party[0].begin_dialog( attachee, 290 )
						else:
							obj.turn_towards(attachee)
							attachee.turn_towards(obj)
							obj.begin_dialog( attachee, 290 )
				elif ( game.quests[58].state >= qs_accepted ):
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 480 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 480 )
				elif ( attachee.has_met(obj) ):
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 490 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 490 )
				else:
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 1 )
					else:
						obj.turn_towards(attachee)
						attachee.turn_towards(obj)
						obj.begin_dialog( attachee, 1 )
				game.global_flags[812] = 1
	return RUN_DEFAULT


def talk_Romag( attachee, triggerer, line):
	npc = find_npc_near(attachee,8037)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,520)
	return SKIP_DEFAULT


def summon_Iuz( attachee, triggerer ):
	print "Hedrack: Summoning Iuz"
	# needs to make Iuz appear near him
	Iuz = game.obj_create(14266, attachee.location-4)
	attachee.turn_towards(Iuz)
	Iuz.turn_towards(attachee)
	return SKIP_DEFAULT


def talk_Iuz( attachee, triggerer, line):
	Iuz = find_npc_near(attachee,8042)
	if (Iuz != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(Iuz,line)
		Iuz.turn_towards(attachee)
		attachee.turn_towards(Iuz)
	else:
		triggerer.begin_dialog(attachee,30)
	return SKIP_DEFAULT


def end_game( attachee, triggerer ):
	game.global_flags[339] = 1
	# play slides and end game
	set_join_slides( attachee, triggerer )
	game.moviequeue_play_end_game()
	return SKIP_DEFAULT


def give_robes( attachee, triggerer ):
	for pc in game.party:
		create_item_in_inventory(6113,pc)
	return SKIP_DEFAULT


def is_28_and_under(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 28):
			return 1
	return 0


def unshit( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT
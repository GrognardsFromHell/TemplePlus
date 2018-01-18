from utilities import *
from toee import *
from Co8 import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (find_npc_near( attachee, 8032 ) != OBJ_HANDLE_NULL):
		return RUN_DEFAULT
	if (game.global_flags[144] == 1):
		attachee.attack(triggerer)
	elif (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 110 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		game.global_vars[720] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[147] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer):
	if (game.global_vars[779] == 0):
		game.global_vars[779] = 2
		attachee.turn_towards(game.party[0])
	elif (game.global_vars[779] == 1 and game.global_flags[823] == 1):
		attachee.remove_from_initiative()
		attachee.object_flag_set(OF_OFF)
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		if (is_daytime() == 0):
			game.global_vars[975] = game.global_vars[975] + 1
			if (game.global_vars[975] == 3 and game.global_flags[373] == 0 and game.global_flags[824] == 0 and game.global_vars[779] == 2 and game.global_flags[823] == 0):
				barky_backup = game.obj_create( 14226, location_from_axis (393L, 545L) )
				barky_backup.move( location_from_axis( 386, 535 ) )
				barky_backup.rotation = 5.49778714378
				game.sound( 4063, 1 )
				barky_backup.unconceal()
				shocky = attachee.get_initiative()
				barky_backup.add_to_initiative()
				barky_backup.set_initiative( shocky )
				game.update_combat_ui()
				for obj in game.obj_list_vicinity(barky_backup.location,OLC_PC):
					barky_backup.attack(obj)
				game.global_flags[824] = 1
			if (game.global_vars[975] == 3 and game.global_flags[374] == 0 and game.global_flags[825] == 0 and game.global_vars[779] == 2 and game.global_flags[823] == 0):
				deggsy_backup = game.obj_create( 14227, location_from_axis (393L, 545L) )
				deggsy_backup.move( location_from_axis( 386, 539 ) )
				deggsy_backup.rotation = 5.49778714378
				if (game.global_flags[373] == 1):
					game.sound( 4063, 1 )
				deggsy_backup.unconceal()
				shocky = attachee.get_initiative()
				deggsy_backup.add_to_initiative()
				deggsy_backup.set_initiative( shocky )
				game.update_combat_ui()
				for obj in game.obj_list_vicinity(deggsy_backup.location,OLC_PC):
					deggsy_backup.attack(obj)
				game.global_flags[825] = 1
		if (obj_percent_hp(attachee) <= 25):
			attachee.object_flag_set(OF_OFF)
			game.particles( "sp-Teleport", attachee )
			game.sound( 4035, 1 )
			game.global_flags[990] = 1
		elif (game.global_vars[743] == 15):
			attachee.obj_set_int(obj_f_critter_strategy, 451)
			game.global_vars[743] = game.global_vars[743] + 1
		elif (game.global_vars[743] >= 16):
			attachee.obj_set_int(obj_f_critter_strategy, 452)	
		else:
			attachee.obj_set_int(obj_f_critter_strategy, 461)
			game.global_vars[743] = game.global_vars[743] + 1
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[147] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.map == 5080):
		if (game.global_flags[147] == 1 or game.global_flags[990] == 1):
			attachee.object_flag_set(OF_OFF)
			return SKIP_DEFAULT
		else:
			if (not game.combat_is_active()):
				closest_jones = party_closest(attachee)
				if ( attachee.distance_to(closest_jones) <= 100 ):
					game.global_vars[720] = game.global_vars[720] + 1
					if (attachee.leader_get() == OBJ_HANDLE_NULL):
						if (game.global_vars[720] == 4):
							attachee.cast_spell(spell_stoneskin, attachee)
							attachee.spells_pending_to_memorized()
						if (game.global_vars[720] == 8):
							attachee.cast_spell(spell_see_invisibility, attachee)
							attachee.spells_pending_to_memorized()
						if (game.global_vars[720] == 12):
							attachee.cast_spell(spell_false_life, attachee)
							attachee.spells_pending_to_memorized()
						if (game.global_vars[720] == 16):
							attachee.cast_spell(spell_mage_armor, attachee)
							attachee.spells_pending_to_memorized()
					if (game.global_vars[720] >= 400):
						game.global_vars[720] = 0
				if (game.global_flags[144] == 0):
					if (game.global_flags[376] == 0):
						for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
							if (is_22_and_under(attachee, obj)):
								obj.turn_towards(attachee)
								attachee.turn_towards(obj)
								if (find_npc_near( attachee, 8035 ) != OBJ_HANDLE_NULL):
									barky = find_npc_near( attachee, 8035 )
									barky.turn_towards(obj)
								if (find_npc_near( attachee, 8036 ) != OBJ_HANDLE_NULL):
									deggy = find_npc_near( attachee, 8036 )
									deggy.turn_towards(obj)
								obj.begin_dialog(attachee,1)
								game.global_flags[376] = 1
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (triggerer.type == obj_t_pc):
		if (game.global_flags[144] == 1):
			return RUN_DEFAULT
	return SKIP_DEFAULT


def senshock_kills_hedrack( attachee, triggerer ):
	game.timevent_add( senshock_check_kill, ( attachee, ), 7200000 )
	return RUN_DEFAULT


def senshock_check_kill( attachee ):
	if (game.global_flags[146] == 0):
		if (game.global_flags[147] == 0):
			attachee.object_flag_set(OF_OFF)
			game.global_flags[146] = 1
	return RUN_DEFAULT


def is_22_and_under(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 20):
			return 1
	return 0
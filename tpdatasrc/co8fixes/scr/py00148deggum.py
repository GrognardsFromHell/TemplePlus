from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if (game.global_flags[144] == 1):
	## temple on alert
		attachee.attack(triggerer)
	elif (not attachee.has_met(triggerer)):
	## haven't met
		triggerer.begin_dialog( attachee, 1 )
	elif (game.global_flags[165] == 1):
	##
		triggerer.begin_dialog( attachee, 150 )
	else:
		triggerer.begin_dialog( attachee, 260 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		game.global_vars[728] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[374] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer):
	if (game.global_vars[779] == 0):
		game.global_vars[779] = 1
		attachee.turn_towards(game.party[0])
	elif (game.global_vars[779] == 2 and game.global_flags[825] == 1):
		attachee.remove_from_initiative()
		attachee.object_flag_set(OF_OFF)
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		if (game.global_flags[373] == 1):
			if (is_daytime() == 0):
				game.global_vars[742] = game.global_vars[742] + 1
				if (game.global_vars[742] == 3 and game.global_flags[147] == 0 and game.global_flags[990] == 0 and game.global_flags[823] == 0 and game.global_vars[779] == 1 and game.global_flags[825] == 0):
					shocky_backup = game.obj_create( 14233, location_from_axis (415L, 528L) )
					shocky_backup.rotation = 3.14159265359
					game.sound( 4035, 1 )
					game.particles( "sp-Teleport", shocky_backup )
					deggsy = attachee.get_initiative()
					shocky_backup.add_to_initiative()
					shocky_backup.set_initiative( deggsy )
					game.update_combat_ui()
					for obj in game.obj_list_vicinity(shocky_backup.location,OLC_PC):
						shocky_backup.attack(obj)
					game.global_flags[823] = 1
		if (obj_percent_hp(attachee) <= 50):
			if (game.global_vars[735] <= 3):
				attachee.obj_set_int(obj_f_critter_strategy, 443)
				game.global_vars[735] = game.global_vars[735] + 1
			elif (game.global_vars[736] <= 9):
				attachee.obj_set_int(obj_f_critter_strategy, 225)
				game.global_vars[736] = game.global_vars[736] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 444)
		else:
			if (game.global_vars[736] <= 9):
				attachee.obj_set_int(obj_f_critter_strategy, 225)
				game.global_vars[736] = game.global_vars[736] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 444)
	return RUN_DEFAULT


##########################################################################################
##	SCRIPT DETAIL FOR START COMBAT							##
##########################################################################################
##	if not dead, unconscious, or prone						##
##		if under 50% health							##
##			if haven't used all 4 healing options (potions and spells)	##
##				set strategy to healing (potions and spells)		##
##				increment healing variable				##
##			otherwise, if haven't cast all 10 normal spells			##
##				set strategy to normal casting				##
##				increment normal casting variable			##
##			otherwise (if have cast all healing and normal spells)		##
##				set strategy to melee					##
##		otherwise (if over 50% health)						##
##			if haven't cast all 10 normal spells				##
##				set strategy to normal casting				##
##				increment normal casting variable			##
##			otheriwse (if have cast all normal spells)			##
##				set strategy to melee					##
##	run default									##
##########################################################################################


def san_resurrect( attachee, triggerer ):
	game.global_flags[374] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_flags[374] == 1):
			attachee.object_flag_set(OF_OFF)
			return SKIP_DEFAULT
		closest_jones = party_closest(attachee)
		if (attachee.distance_to(closest_jones) <= 100):
			game.global_vars[728] = game.global_vars[728] + 1
			if (attachee.leader_get() == OBJ_HANDLE_NULL):
				if (game.global_vars[728] == 4):
					attachee.cast_spell(spell_shield_of_faith, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[728] == 8):
					attachee.cast_spell(spell_blur, attachee)
					attachee.spells_pending_to_memorized()
			if (game.global_vars[728] >= 400):
				game.global_vars[728] = 0
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (triggerer.type == obj_t_pc):
		if (game.global_flags[144] == 1):
			return RUN_DEFAULT
	return SKIP_DEFAULT



def banter( attachee, triggerer, line):
	npc = find_npc_near(attachee,8035)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,90)
	return SKIP_DEFAULT


def banter2( attachee, triggerer, line):
	npc = find_npc_near(attachee,8035)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,80)
	return SKIP_DEFAULT
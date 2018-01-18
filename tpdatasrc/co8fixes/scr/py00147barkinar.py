from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_flags[144] == 1):
		attachee.attack(triggerer)
	elif (not attachee.has_met(triggerer)):
		if (anyone(triggerer.group_list(),"has_wielded",3010) or anyone(triggerer.group_list(),"has_wielded",3016) or anyone(triggerer.group_list(),"has_wielded",3017) or anyone(triggerer.group_list(),"has_wielded",3020)):
			triggerer.begin_dialog( attachee, 130 )
		else:
			triggerer.begin_dialog( attachee, 1 )
	elif (game.global_flags[163] == 1):
		triggerer.begin_dialog( attachee, 220 )
	elif (game.global_flags[157] == 1):
		if ((game.global_flags[146] == 1) and (game.global_flags[147] == 1) and (game.global_flags[153] == 0) and (game.global_flags[156] == 0)):
			triggerer.begin_dialog( attachee, 140 )
		else:
			triggerer.begin_dialog( attachee, 150 )
	elif (game.global_flags[162] == 1):
		triggerer.begin_dialog( attachee, 160 )
	elif (game.global_flags[158] == 1):
		triggerer.begin_dialog( attachee, 190 )
	elif (anyone(triggerer.group_list(),"has_wielded",3010) or anyone(triggerer.group_list(),"has_wielded",3016) or anyone(triggerer.group_list(),"has_wielded",3017) or anyone(triggerer.group_list(),"has_wielded",3020)):	
		triggerer.begin_dialog( attachee, 130 )	
	else:
		triggerer.begin_dialog( attachee, 210 )	
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		game.global_vars[725] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[373] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer):
	if (game.global_vars[779] == 0):
		game.global_vars[779] = 1
		attachee.turn_towards(game.party[0])
	elif (game.global_vars[779] == 2 and game.global_flags[824] == 1):
		attachee.remove_from_initiative()
		attachee.object_flag_set(OF_OFF)
		return SKIP_DEFAULT
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		if (is_daytime() == 0):
			game.global_vars[742] = game.global_vars[742] + 1
			if (game.global_vars[742] == 3 and game.global_flags[147] == 0 and game.global_flags[990] == 0 and game.global_flags[823] == 0 and game.global_vars[779] == 1 and game.global_flags[824] == 0):
				shocky_backup = game.obj_create( 14233, location_from_axis (415L, 528L) )
				shocky_backup.rotation = 3.14159265359
				game.sound( 4035, 1 )
				game.particles( "sp-Teleport", shocky_backup )
				barky = attachee.get_initiative()
				shocky_backup.add_to_initiative()
				shocky_backup.set_initiative( barky )
				game.update_combat_ui()
				for obj in game.obj_list_vicinity(shocky_backup.location,OLC_PC):
					shocky_backup.attack(obj)
				game.global_flags[823] = 1
		if (obj_percent_hp(attachee) <= 33):
			game.global_vars[741] = game.global_vars[741] + 1
			if (game.global_vars[741] >= 3 and game.global_flags[242] == 0):
				chance = game.random_range(1,3)
				if (chance == 1):
					if (is_daytime() == 0):
						bugbear_backup_1 = game.obj_create( 14826, location_from_axis (437L, 545L) )
						bugbear_backup_1.rotation = 0.78539816340
						bugbear_backup_1.unconceal()
						bugbear_backup_2 = game.obj_create( 14826, location_from_axis (433L, 545L) )
						bugbear_backup_2.rotation = 0.78539816340
						bugbear_backup_2.unconceal()
						game.sound( 4063, 1 )
						barky = attachee.get_initiative()
						bugbear_backup_1.add_to_initiative()
						bugbear_backup_2.add_to_initiative()
						bugbear_backup_1.set_initiative( barky )
						bugbear_backup_2.set_initiative( barky )
						game.update_combat_ui()
						for obj in game.obj_list_vicinity(bugbear_backup_1.location,OLC_PC):
							bugbear_backup_1.attack(obj)
						for obj in game.obj_list_vicinity(bugbear_backup_2.location,OLC_PC):
							bugbear_backup_2.attack(obj)
						game.global_flags[242] = 1
					elif (is_daytime() == 1):
						bugbear_backup_1 = game.obj_create( 14826, location_from_axis (386L, 540L) )
						bugbear_backup_1.rotation = 5.49778714378
						bugbear_backup_1.unconceal()
						bugbear_backup_2 = game.obj_create( 14826, location_from_axis (386L, 536L) )
						bugbear_backup_2.rotation = 5.49778714378
						bugbear_backup_2.unconceal()
						game.sound( 4063, 1 )
						barky = attachee.get_initiative()
						bugbear_backup_1.add_to_initiative()
						bugbear_backup_2.add_to_initiative()
						bugbear_backup_1.set_initiative( barky )
						bugbear_backup_2.set_initiative( barky )
						game.update_combat_ui()
						for obj in game.obj_list_vicinity(bugbear_backup_1.location,OLC_PC):
							bugbear_backup_1.attack(obj)
						for obj in game.obj_list_vicinity(bugbear_backup_2.location,OLC_PC):
							bugbear_backup_2.attack(obj)
						game.global_flags[242] = 1
			if (game.global_vars[740] <= 1):
				attachee.obj_set_int(obj_f_critter_strategy, 450)
				game.global_vars[740] = game.global_vars[740] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 465)
		elif (obj_percent_hp(attachee) >= 34 and obj_percent_hp(attachee) <= 67):
			if (game.global_vars[737] <= 2):
				attachee.obj_set_int(obj_f_critter_strategy, 445)
				game.global_vars[737] = game.global_vars[737] + 1
			elif (game.global_vars[738] <= 5):
				attachee.obj_set_int(obj_f_critter_strategy, 442)
				game.global_vars[738] = game.global_vars[738] + 1
			elif (game.global_vars[739] <= 2):
				if (find_npc_near( attachee, 8036 ) != OBJ_HANDLE_NULL or find_npc_near( attachee, 8729 ) != OBJ_HANDLE_NULL):
					deggum = find_npc_near( attachee, 8036 )
					senshock = find_npc_near( attachee, 8729 )
					if (obj_percent_hp(deggum) <= 50 or obj_percent_hp(senshock) <= 50):
						attachee.obj_set_int(obj_f_critter_strategy, 447)
						game.global_vars[739] = game.global_vars[739] + 1
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 446)
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 446)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 446)
		else:
			if (game.global_vars[739] <= 2):
				if (find_npc_near( attachee, 8036 ) != OBJ_HANDLE_NULL or find_npc_near( attachee, 8729 ) != OBJ_HANDLE_NULL):
					deggum = find_npc_near( attachee, 8036 )
					senshock = find_npc_near( attachee, 8729 )
					if (obj_percent_hp(deggum) <= 50 or obj_percent_hp(senshock) <= 50):
						attachee.obj_set_int(obj_f_critter_strategy, 447)
						game.global_vars[739] = game.global_vars[739] + 1
					elif (game.global_vars[738] <= 5):
						attachee.obj_set_int(obj_f_critter_strategy, 442)
						game.global_vars[738] = game.global_vars[738] + 1
					else:
						attachee.obj_set_int(obj_f_critter_strategy, 446)
				elif (game.global_vars[738] <= 5):
					attachee.obj_set_int(obj_f_critter_strategy, 442)
					game.global_vars[738] = game.global_vars[738] + 1
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 446)
			elif (game.global_vars[738] <= 5):
				attachee.obj_set_int(obj_f_critter_strategy, 442)
				game.global_vars[738] = game.global_vars[738] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 446)
	return RUN_DEFAULT


##################################################################################################
##	SCRIPT DETAIL FOR START COMBAT								##
##################################################################################################
##	if not dead, unconscious, or prone							##
##		if under 33% health								##
##			increment bugbear backup variable					##
##			if 4 or more turns have passed and bugbears have not been spawned	##
##				generate 1 in 3 chance						##
##				if chance is met						##
##					spawn bugbear backup					##
##			if haven't cast all 2 self protection spells				##
##				set strategy to self protection					##
##				increment self protection variable				##
##			otherwise								##
##				set strategy to defense						##
##		otherwise, if between 34% and 67% health					##
##			if haven't cast all 3 self healing spells				##
##				set strategy to self healing					##
##				increment self healing variable					##
##			otherwise, if haven't cast all 6 normal spells				##
##				set strategy to normal casting					##
##				increment normal casting variable				##
##			otherwise, if haven't cast all 3 friend healing spells			##
##				if deggum or senshock are present and not dead			##
##					if either of them are under 50% health			##
##						set strategy to friend healing			##
##						increment friend healing variable		##
##					otherwise						##
##						set strategy to melee				##
##				otherwise							##
##					set strategy to melee					##
##			otherwise (if have cast all protection, healing and normal spells)	##
##				set strategy to melee						##
##		otherwise (if over 66% health)							##
##			if haven't cast all 3 friend healing spells				##
##				if deggum or senshock are present and not dead			##
##					if either of them are under 50% health			##
##						set strategy to friend healing			##
##						increment friend healing variable		##
##					otherwise, if haven't cast all 6 normal spells		##
##						set strategy to normal casting			##
##						increment normal casting variable		##
##					otherwise						##
##						set strategy to melee				##
##				otherwise, if haven't cast all 6 normal spells			##
##					set strategy to normal casting				##
##					increment normal casting variable			##
##				otherwise							##
##					set strategy to melee					##
##			otherwise, if haven't cast all 6 normal spells				##
##				set strategy to normal casting					##
##				increment normal casting variable				##
##			otheriwse (if have cast all friend healing and normal spells)		##
##				set strategy to melee						##
##	run default										##
##################################################################################################


def san_resurrect( attachee, triggerer ):
	game.global_flags[373] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_flags[373] == 1):
			attachee.object_flag_set(OF_OFF)
			return SKIP_DEFAULT
		closest_jones = party_closest(attachee)
		if (attachee.distance_to(closest_jones) <= 100):
			game.global_vars[725] = game.global_vars[725] + 1
			if (attachee.leader_get() == OBJ_HANDLE_NULL):
				if (game.global_vars[725] == 4):
					attachee.cast_spell(spell_shield_of_faith, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[725] == 8):
					attachee.cast_spell(spell_blur, attachee)
					attachee.spells_pending_to_memorized()
				if (game.global_vars[725] == 12):
					attachee.cast_spell(spell_magic_circle_against_good, attachee)
					attachee.spells_pending_to_memorized()
			if (game.global_vars[725] >= 400):
				game.global_vars[725] = 0
		if (game.global_flags[144] == 0):
			if (game.global_flags[375] == 0):
				if (is_daytime() == 0):
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_28_and_under(attachee, obj)):
							obj.turn_towards(attachee)
							attachee.turn_towards(obj)
							if (find_npc_near( attachee, 8036 ) != OBJ_HANDLE_NULL):
								deggum = find_npc_near( attachee, 8036 )
								deggum.turn_towards(obj)
							obj.begin_dialog(attachee,1)
							game.global_flags[375] = 1
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (triggerer.type == obj_t_pc):
		if (game.global_flags[144] == 1):
			return RUN_DEFAULT
	return SKIP_DEFAULT


def banter( attachee, triggerer, line):
	npc = find_npc_near(attachee,8036)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,70)
	return SKIP_DEFAULT


def banter2( attachee, triggerer, line):
	npc = find_npc_near(attachee,8036)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,80)
	return SKIP_DEFAULT


def is_28_and_under(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 28):
			return 1
	return 0
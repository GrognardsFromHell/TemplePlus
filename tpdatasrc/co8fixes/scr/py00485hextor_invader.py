#### Added by Ranth for High Level Expansion
from toee import *
from utilities import *
from co8 import *
from combat_standard_routines import *


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.name == 8737):
		destroy_gear( attachee, triggerer )
		game.global_vars[511] = game.global_vars[511] + 1
		if (game.global_vars[511] >= 24 and game.global_flags[501] == 1):
			game.global_flags[511] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8738):
		destroy_gear( attachee, triggerer )
		game.global_vars[512] = game.global_vars[512] + 1
		if (game.global_vars[512] >= 24):
			game.global_flags[512] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8739):
		destroy_gear( attachee, triggerer )
		game.global_vars[513] = game.global_vars[513] + 1
		if (game.global_vars[513] >= 24):
			game.global_flags[513] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8740):
		destroy_gear( attachee, triggerer )
		game.global_vars[514] = game.global_vars[514] + 1
		if (game.global_vars[514] >= 24):
			game.global_flags[514] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8741):
		destroy_gear( attachee, triggerer )
		game.global_vars[515] = game.global_vars[515] + 1
		if (game.global_vars[515] >= 24):
			game.global_flags[515] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8742):
		destroy_gear( attachee, triggerer )
		game.global_vars[516] = game.global_vars[516] + 1
		if (game.global_vars[516] >= 12):
			game.global_flags[516] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8743):
		destroy_gear( attachee, triggerer )
		game.global_vars[517] = game.global_vars[517] + 1
		if (game.global_vars[517] >= 12):
			game.global_flags[517] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8744):
		destroy_gear( attachee, triggerer )
		game.global_vars[518] = game.global_vars[518] + 1
		if (game.global_vars[518] >= 12):
			game.global_flags[518] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8745):
		destroy_gear( attachee, triggerer )
		game.global_vars[519] = game.global_vars[519] + 1
		if (game.global_vars[519] >= 12):
			game.global_flags[519] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8746):
		destroy_gear( attachee, triggerer )
		game.global_vars[520] = game.global_vars[520] + 1
		if (game.global_vars[520] >= 5):
			game.global_flags[520] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8747):
		destroy_gear( attachee, triggerer )
		game.global_vars[521] = game.global_vars[521] + 1
		if (game.global_vars[521] >= 6):
			game.global_flags[521] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	if (attachee.name == 8748):
		destroy_gear( attachee, triggerer )
		game.global_vars[522] = game.global_vars[522] + 1
		if (game.global_vars[522] >= 6):
			game.global_flags[522] = 1
			if (game.global_flags[511] == 1 and game.global_flags[512] == 1 and game.global_flags[513] == 1 and game.global_flags[514] == 1 and game.global_flags[515] == 1 and game.global_flags[516] == 1 and game.global_flags[517] == 1 and game.global_flags[518] == 1 and game.global_flags[519] == 1 and game.global_flags[520] == 1 and game.global_flags[521] == 1 and game.global_flags[522] == 1):
				game.quests[97].state = qs_completed
				game.party[0].reputation_add( 52 )
				game.global_vars[501] = 7
			else:
				game.sound( 4132, 2 )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (game.global_vars[505] == 0):
		game.timevent_add( out_of_time, ( attachee, triggerer ), 7200000 )	# 2 hours
		game.global_vars[505] = 1
	if (triggerer.type == obj_t_pc):
		if anyone( triggerer.group_list(), "has_follower", 8736 ):
			wakefield = find_npc_near( triggerer, 8736 )
			if (wakefield != OBJ_HANDLE_NULL):
				triggerer.follower_remove(wakefield)
				wakefield.float_line( 20000,triggerer )
				wakefield.attack(triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee.name == 8738):
		attachee.obj_set_int(obj_f_critter_strategy, 436)
	elif (attachee.name == 8739):
		attachee.obj_set_int(obj_f_critter_strategy, 437)
	elif (attachee.name == 8740):
		attachee.obj_set_int(obj_f_critter_strategy, 438)
	elif (attachee.name == 8741):
		attachee.obj_set_int(obj_f_critter_strategy, 439)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.quests[97].state == qs_botched):
		attachee.object_flag_set(OF_OFF)
	if (attachee.name == 8738):
		if (game.global_flags[509] == 0):
			game.timevent_add( tower_attack, ( attachee, triggerer ), 6000 )
			game.global_flags[509] = 1
	elif (attachee.name == 8739):
		if (game.global_flags[510] == 0):
			game.timevent_add( church_attack, ( attachee, triggerer ), 6000 )
			game.global_flags[510] = 1
	elif (attachee.name == 8740):
		if (game.global_flags[523] == 0):
			game.timevent_add( grove_attack, ( attachee, triggerer ), 6000 )
			game.global_flags[523] = 1
	elif (attachee.name == 8741):
		if (game.global_flags[524] == 0):
			game.timevent_add( wench_attack, ( attachee, triggerer ), 6000 )
			game.global_flags[524] = 1
	float_select = game.random_range(1,6)
	if (attachee.scripts[san_dialog]):
		attachee.float_line(float_select,triggerer)
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_flags[525] == 1):
		return SKIP_DEFAULT
	else:
		return RUN_DEFAULT


def destroy_gear( attachee, triggerer ):
	fighter_longsword = attachee.item_find(4132)
	fighter_longsword.destroy()
	fighter_towershield = attachee.item_find(6078)
	fighter_towershield.destroy()
	soldier_shield = attachee.item_find(6068)
	soldier_shield.destroy()
	cleric_crossbow = attachee.item_find(4178)
	cleric_crossbow.destroy()
	archer_longbow = attachee.item_find(4087)
	archer_longbow.destroy()
	gold_breastplate = attachee.item_find(6477)
	gold_breastplate.destroy()
	gold_chainmail = attachee.item_find(6476)
	gold_chainmail.destroy()
	plain_chainmail = attachee.item_find(6454)
	plain_chainmail.destroy()
	red_chainmail = attachee.item_find(6019)
	red_chainmail.destroy()
	fine_chainmail = attachee.item_find(6475)
	fine_chainmail.destroy()
	splintmail = attachee.item_find(6096)
	splintmail.destroy()
	black_bandedmail = attachee.item_find(6341)
	black_bandedmail.destroy()
	silver_bandedmail = attachee.item_find(6120)
	silver_bandedmail.destroy()
	halfplate = attachee.item_find(6158)
	halfplate.destroy()
	return


def out_of_time( attachee, triggerer ):
	game.global_vars[505] = 3
	return


def tower_attack( attachee, triggerer ):
	game.particles( "sp-Fireball-Hit", location_from_axis( 455, 609 ) )
	game.particles( "ef-fireburning", location_from_axis( 455, 609 ) )
	game.particles( "ef-FirePit", location_from_axis( 455, 609 ) )
	game.particles( "sp-Fireball-Hit", location_from_axis( 439, 610 ) )
	game.particles( "ef-fireburning", location_from_axis( 439, 610 ) )
	game.particles( "ef-FirePit", location_from_axis( 439, 610 ) )
	game.sound( 4134, 1 )
	game.shake(75,3200)
	game.timevent_add( tower_attack_followup, (), 12000 )
	return RUN_DEFAULT


def church_attack( attachee, triggerer ):
	game.particles( "sp-Fireball-Hit", location_from_axis( 490, 224 ) )
	game.particles( "ef-fireburning", location_from_axis( 490, 224 ) )
	game.particles( "ef-FirePit", location_from_axis( 490, 224 ) )
	game.particles( "sp-Fireball-Hit", location_from_axis( 506, 217 ) )
	game.particles( "ef-fireburning", location_from_axis( 506, 217 ) )
	game.particles( "ef-FirePit", location_from_axis( 506, 217 ) )
	game.sound( 4135, 1 )
	game.shake(75,3200)
	game.timevent_add( church_attack_followup, (), 12000 )
	return RUN_DEFAULT


def grove_attack( attachee, triggerer ):
	game.particles( "sp-Fireball-Hit", location_from_axis( 617, 523 ) )
	game.particles( "ef-fireburning", location_from_axis( 617, 523 ) )
	game.particles( "ef-FirePit", location_from_axis( 617, 523 ) )
	game.particles( "sp-Fireball-Hit", location_from_axis( 616, 515 ) )
	game.particles( "ef-fireburning", location_from_axis( 616, 515 ) )
	game.particles( "ef-FirePit", location_from_axis( 616, 515 ) )
	game.sound( 4136, 1 )
	game.shake(75,3200)
	game.timevent_add( grove_attack_followup, (), 12000 )
	return RUN_DEFAULT


def wench_attack( attachee, triggerer ):
	game.particles( "sp-Fireball-Hit", location_from_axis( 621, 397 ) )
	game.particles( "ef-fireburning", location_from_axis( 621, 397 ) )
	game.particles( "ef-FirePit", location_from_axis( 621, 397 ) )
	game.particles( "sp-Fireball-Hit", location_from_axis( 609, 399 ) )
	game.particles( "ef-fireburning", location_from_axis( 609, 399 ) )
	game.particles( "ef-FirePit", location_from_axis( 609, 399 ) )
	game.sound( 4136, 1 )
	game.shake(75,3200)
	game.timevent_add( wench_attack_followup, (), 12000 )
	return RUN_DEFAULT


def tower_attack_followup():
	if (not game.combat_is_active()):
		random_x = game.random_range(428,465)
		random_y = game.random_range(597,617)
		game.particles( "sp-Fireball-Hit", location_from_axis( random_x, random_y ) )
		game.particles( "ef-fireburning", location_from_axis( random_x, random_y ) )
		game.particles( "ef-FirePit", location_from_axis( random_x, random_y ) )
		game.sound( 4135, 1 )
		game.shake(50,1600)
		game.timevent_add( tower_attack_followup, (), 12000 )
	return RUN_DEFAULT


def church_attack_followup():
	if (not game.combat_is_active()):
		random_x = game.random_range(478,509)
		random_y = game.random_range(207,235)
		game.particles( "sp-Fireball-Hit", location_from_axis( random_x, random_y ) )
		game.particles( "ef-fireburning", location_from_axis( random_x, random_y ) )
		game.particles( "ef-FirePit", location_from_axis( random_x, random_y ) )
		game.sound( 4135, 1 )
		game.shake(50,1600)
		game.timevent_add( church_attack_followup, (), 12000 )
	return RUN_DEFAULT


def grove_attack_followup():
	if (not game.combat_is_active()):
		random_x = game.random_range(593,621)
		random_y = game.random_range(508,538)
		game.particles( "sp-Fireball-Hit", location_from_axis( random_x, random_y ) )
		game.particles( "ef-fireburning", location_from_axis( random_x, random_y ) )
		game.particles( "ef-FirePit", location_from_axis( random_x, random_y ) )
		game.sound( 4135, 1 )
		game.shake(50,1600)
		game.timevent_add( grove_attack_followup, (), 12000 )
	return RUN_DEFAULT


def wench_attack_followup():
	if (not game.combat_is_active()):
		random_x = game.random_range(590,641)
		random_y = game.random_range(370,404)
		game.particles( "sp-Fireball-Hit", location_from_axis( random_x, random_y ) )
		game.particles( "ef-fireburning", location_from_axis( random_x, random_y ) )
		game.particles( "ef-FirePit", location_from_axis( random_x, random_y ) )
		game.sound( 4135, 1 )
		game.shake(50,1600)
		game.timevent_add( wench_attack_followup, (), 12000 )
	return RUN_DEFAULT
from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	st = attachee.obj_get_int( obj_f_npc_pad_i_5 )
	attachee.turn_towards(triggerer)
	if (attachee.map == 5013):
		attachee.float_line( 23000, triggerer )
	elif (game.global_vars[915] == 32):
		triggerer.begin_dialog( attachee, 570 )			## have attacked 3 or more farm animals with ronald in party
	elif (game.leader.reputation_has(32) == 1 or game.leader.reputation_has(30) == 1 or game.leader.reputation_has(29) == 1):
		attachee.float_line(11004,triggerer)
	elif (attachee.has_met( triggerer )):
		if game.global_vars[692] == 2 and game.global_vars[693] == 3 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			attachee.float_line(600,triggerer)		## not in party, Gruumsh
			return SKIP_DEFAULT
		elif game.global_vars[692] == 1 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,260)		## not in party, Lareth
			return SKIP_DEFAULT
		elif game.global_vars[692] == 3 and (game.global_vars[693] == 1 or game.global_vars[692] == 2) and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,90)		## not in party, orcs present after issues
			return SKIP_DEFAULT
		elif game.global_vars[692] == 3 and game.global_vars[693] == 0 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,355)		## not in party, orcs gone after issues
			return SKIP_DEFAULT
		elif (game.global_vars[692] >= 5 and game.global_vars[692] <= 7) and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,100)		## return with money
			return SKIP_DEFAULT
		elif game.global_vars[692] == 0 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,200)		## return with no grudges
			return SKIP_DEFAULT
		elif game.global_vars[692] == 2 and game.global_vars[693] == 0 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,355)		## not in party, returning after Gruumsh, Gruumsh gone
			return SKIP_DEFAULT
		elif game.global_vars[692] == 8 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee,900)		## not in party, left because party didn't pay for resurrection - not done yet!!!!
			return SKIP_DEFAULT	
		elif game.global_vars[692] == 9 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			attachee.float_line(600,triggerer)		## left because hates party's guts, won't talk
			return SKIP_DEFAULT
		elif game.global_vars[692] == 10 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee, 2150)		## done, left to talk to Dad
			return SKIP_DEFAULT
		elif game.global_vars[692] == 11 and (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog(attachee, 2200)		## chatted with Dad, ready to go
			return SKIP_DEFAULT
		elif (attachee.leader_get() != OBJ_HANDLE_NULL and st == 1):
			triggerer.begin_dialog(attachee,2060)		## in party, wanting to resurrect
			return SKIP_DEFAULT
		elif (attachee.leader_get() != OBJ_HANDLE_NULL and st == 0):
			triggerer.begin_dialog(attachee,120)		## in party, not resurrecting yet
			return SKIP_DEFAULT
		elif (attachee.leader_get() != OBJ_HANDLE_NULL and st == 2):
			triggerer.begin_dialog(attachee,2250)		## back in party, post parents
			return SKIP_DEFAULT
		else:
			attachee.float_line(600,triggerer)		## anything else he just won't talk
			return SKIP_DEFAULT
	else:
		if (game.global_vars[5] > 7) and triggerer.reputation_has( 2 ) == 0:
			attachee.float_line(600,triggerer)
		else:
			triggerer.begin_dialog( attachee, 1 )		## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[500] == 0):
		attachee.object_flag_set(OF_OFF)
	else:
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			if (attachee.map == 5011):
				if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
					attachee.object_flag_set(OF_OFF)
				else:
					attachee.object_flag_unset(OF_OFF)
			elif (attachee.map == 5013):
				if (game.global_vars[510] != 2):
					if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6):
						attachee.object_flag_unset(OF_OFF)
				else:
					attachee.object_flag_set(OF_OFF)
		finale = attachee.obj_get_int( obj_f_npc_pad_i_5 )
		if ((anyone( game.party[0].group_list(), "has_follower", 8025 )) or (anyone( game.party[0].group_list(), "has_follower", 8026 ))):    # Tuelk and Pintark
			game.global_vars[693] = 2
		half_orc = 0
		Grummshite = 0
		for pc in game.party:
			if pc.stat_level_get(stat_race) == race_halforc:
				half_orc = 1
			if (pc.stat_level_get( stat_deity ) == 7 and pc.stat_level_get(stat_level_cleric) >= 1):
				Grummshite = 1
		if half_orc == 1:
			game.global_vars[693] = 1
		if Grummshite == 1:
			game.global_vars[693] = 3
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	if (game.global_flags[237] == 0):
		game.global_vars[23] = game.global_vars[23] + 1
		if game.global_vars[23] >= 2:
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	attachee.float_line(12057,triggerer)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		if (game.global_vars[915] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_taking_damage( attachee, triggerer ):
	attachee.float_line(12054,triggerer)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	game.global_flags[237] = 1
	itemD = attachee.item_find(4068)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.item_flag_set(OIF_NO_TRANSFER)
	itemE = attachee.item_find(4087)
	if (itemE != OBJ_HANDLE_NULL):
		itemE.item_flag_set(OIF_NO_TRANSFER)
	itemA = attachee.item_find(6056)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.item_flag_set(OIF_NO_TRANSFER)
	itemB = attachee.item_find(6070)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.item_flag_set(OIF_NO_TRANSFER)
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[237] = 0
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):		# after moathouse, sets to 283
	if (attachee.map == 5004):
		attachee.float_line(650,triggerer)
		game.new_sid = 283
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_cats_grace or spell.spell == spell_invisibility ):
		attachee.float_line(630,triggerer)
	elif ( spell.spell == spell_haste or spell.spell == spell_good_hope ):
		attachee.float_line(630,triggerer)
	elif ( spell.spell == spell_stoneskin or spell.spell == spell_barkskin ):
		attachee.float_line(630,triggerer)
	elif ( spell.spell == spell_blur or spell.spell == spell_foxs_cunning ):
		attachee.float_line(630,triggerer)
	elif ( spell.spell == spell_heroism or spell.spell == spell_mage_armor ):
		attachee.float_line(630,triggerer)
	elif ( spell.spell == spell_mirror_image or spell.spell == spell_protection_from_arrows ):
		attachee.float_line(630,triggerer)
	return RUN_DEFAULT


def run_off( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
	attachee.runoff(attachee.location-3)
	return RUN_DEFAULT


def argue_lareth( attachee, triggerer, line):
	npc = find_npc_near(attachee,8002)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(triggerer)
		triggerer.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,270)
	return SKIP_DEFAULT


def war( attachee, triggerer ):
	hedrack = find_npc_near(attachee,8032)
	iuz = find_npc_near(attachee,8042)
	if (hedrack != OBJ_HANDLE_NULL):
		hedrack.attack( triggerer )
	elif (iuz != OBJ_HANDLE_NULL):
		iuz.attack( triggerer )
	return SKIP_DEFAULT


def switch_to_cuthbert( attachee, triggerer, line ):
	cuthbert = find_npc_near(attachee,8043)
	if (cuthbert != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(cuthbert,line)
	else:
		cuthbert.object_flag_set(OF_OFF)
	return SKIP_DEFAULT


def argue_tuelk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8026)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(triggerer)
		triggerer.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,175)
	return SKIP_DEFAULT


def reset_st(attachee, triggerer):
	attachee.obj_set_int( obj_f_npc_pad_i_5, 1 )
	return SKIP_DEFAULT


def summon_parents( attachee, triggerer ):
	willi = game.obj_create(14676, attachee.location-3)
	game.particles( 'sp-Raise Dead', willi)
	willi.turn_towards(attachee)
	ivy = game.obj_create(14675, attachee.location-3)
	game.particles( 'sp-Raise Dead', ivy)
	ivy.turn_towards(attachee)
	attachee.turn_towards(willi)
	return SKIP_DEFAULT


def argue_willi( attachee, triggerer, line):
	willi = find_npc_near(attachee,14676)
	if (willi != OBJ_HANDLE_NULL):
		attachee.turn_towards(triggerer)
		triggerer.begin_dialog(willi,line)
		willi.turn_towards(triggerer)
		triggerer.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,175)
	return SKIP_DEFAULT


def done(attachee, triggerer):
	game.global_vars[692] = 10
	attachee.obj_set_int( obj_f_npc_pad_i_5, 2 )
	return SKIP_DEFAULT


def reunion( attachee, triggerer):
	willi = find_npc_near(attachee,14676)
	willi.critter_flag_set( OCF_MUTE )
	game.timevent_add( rejoin, (attachee, triggerer), 88000000 )
	attachee.critter_flag_set( OCF_MUTE )
	return SKIP_DEFAULT


def rejoin(attachee, triggerer):
	attachee.critter_flag_unset( OCF_MUTE )
	willi = find_npc_near(attachee,14676)
	willi.runoff(attachee.location-3)
	ivy = find_npc_near(attachee,14675)
	ivy.runoff(attachee.location-3)
	game.global_vars[692] = 11
	return SKIP_DEFAULT


def equip_all( attachee, triggerer ):
	create_item_in_inventory( 7002, attachee )
	create_item_in_inventory( 7001, attachee )
	create_item_in_inventory( 7001, attachee )
	itemB = attachee.item_find(6070)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.destroy()
		create_item_in_inventory( 6070, triggerer )
	itemD = attachee.item_find(4068)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.destroy()
		create_item_in_inventory( 4068, triggerer )
	itemE = attachee.item_find(4087)
	if (itemE != OBJ_HANDLE_NULL):
		itemE.destroy()
		create_item_in_inventory( 4087, triggerer )
	itemA = attachee.item_find(6056)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.destroy()
		create_item_in_inventory( 6056, triggerer )
	return RUN_DEFAULT


def equip_leather( attachee, triggerer ):
	itemC = attachee.item_find(6056)
	if (itemC != OBJ_HANDLE_NULL):
		itemC.destroy()
		create_item_in_inventory( 6056, triggerer )
	create_item_in_inventory( 7001, attachee )
	create_item_in_inventory( 7001, attachee )
	create_item_in_inventory( 7001, attachee )
	return RUN_DEFAULT


def equip_rest( attachee, triggerer ):
	create_item_in_inventory( 7002, attachee )
	create_item_in_inventory( 7001, attachee )
	create_item_in_inventory( 7001, attachee )
	itemB = attachee.item_find(6070)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.destroy()
		create_item_in_inventory( 6070, triggerer )
	itemD = attachee.item_find(4068)
	if (itemD != OBJ_HANDLE_NULL):
		itemD.destroy()
		create_item_in_inventory( 4068, triggerer )
	itemE = attachee.item_find(4087)
	if (itemE != OBJ_HANDLE_NULL):
		itemE.destroy()
		create_item_in_inventory( 4087, triggerer )
	return RUN_DEFAULT
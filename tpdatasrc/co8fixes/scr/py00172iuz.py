from toee import *
from Co8 import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_flags[371] == 0):
	## iuz has not talked
		if (triggerer.item_find(2203) != OBJ_HANDLE_NULL):
			triggerer.begin_dialog( attachee, 1 )
		elif (find_npc_near(attachee,8032) != OBJ_HANDLE_NULL):
			triggerer.begin_dialog( attachee, 100 )
		else:
			triggerer.begin_dialog( attachee, 130 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	#if should_modify_CR( attachee ):
	#	modify_CR( attachee, get_av_level() )
	game.global_flags[327] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	print "Iuz start combat"
	game.global_vars[32] = game.global_vars[32] + 1
	if (game.global_flags[328] == 1):
	## cuthbert has already appeared and iuz shouldn't be there
		attachee.remove_from_initiative()
		attachee.object_flag_set(OF_OFF)
		game.particles( "sp-Magic Circle against Good-END", attachee )
		game.sound( 4043, 1 )
		return SKIP_DEFAULT
	else:
	## cuthbert has not appeared
		if (find_npc_near( triggerer, 8032 ) != OBJ_HANDLE_NULL):
		## hedrack is near
			if (game.global_vars[32] >= 4 and attachee.d20_query(Q_Prone) == 0):
			## 4th round of combat or higher and Iuz is not prone
				cuthbert = game.obj_create( 14267, attachee.location-2 )
				game.particles( "hit-LAW-medium", cuthbert )
				attachee.turn_towards(cuthbert)
				cuthbert.turn_towards(attachee)
				StopCombat(attachee, 0)
				for pc in game.party:
					if pc.type == obj_t_pc:
						attachee.ai_shitlist_remove( pc )
				delegatePc = GetDelegatePc(attachee, 35, 0)
				game.sound( 4134, 1 )
				if (delegatePc != OBJ_HANDLE_NULL):
					delegatePc.turn_towards(cuthbert)
					attachee.turn_towards(delegatePc)
					delegatePc.begin_dialog( cuthbert, 1 )
					game.new_sid = 0
					return SKIP_DEFAULT
		strategy = game.random_range(453,460)
		if (strategy == 453):
			attachee.obj_set_int(obj_f_critter_strategy, 453)
		elif (strategy == 454):
			attachee.obj_set_int(obj_f_critter_strategy, 454)
		elif (strategy == 455):
			attachee.obj_set_int(obj_f_critter_strategy, 455)
		elif (strategy == 456):
			attachee.obj_set_int(obj_f_critter_strategy, 456)
		elif (strategy == 457):
			attachee.obj_set_int(obj_f_critter_strategy, 457)
		elif (strategy == 458):
			attachee.obj_set_int(obj_f_critter_strategy, 458)
		elif (strategy == 459):
			attachee.obj_set_int(obj_f_critter_strategy, 459)
		elif (strategy == 460):
			attachee.obj_set_int(obj_f_critter_strategy, 460)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[361] == 0):
		game.global_flags[361] = 1
		# game.particles( "mon-iuz", attachee )
	if (not game.combat_is_active()):
		for pc in game.party:
			if pc.type == obj_t_pc:
				if (anyone( pc.group_list(), "has_item", 2203 )):
				## party has golden skull
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[1].stat_level_get(stat_hp_current) >= 1 and game.party[1].d20_query(Q_Prone) == 0):
						game.party[1].turn_towards(attachee)
						attachee.turn_towards(game.party[1])
						game.party[1].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[2].stat_level_get(stat_hp_current) >= 1 and game.party[2].d20_query(Q_Prone) == 0):
						game.party[2].turn_towards(attachee)
						attachee.turn_towards(game.party[2])
						game.party[2].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[3].stat_level_get(stat_hp_current) >= 1 and game.party[3].d20_query(Q_Prone) == 0):
						game.party[3].turn_towards(attachee)
						attachee.turn_towards(game.party[3])
						game.party[3].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[4].stat_level_get(stat_hp_current) >= 1 and game.party[4].d20_query(Q_Prone) == 0):
						game.party[4].turn_towards(attachee)
						attachee.turn_towards(game.party[4])
						game.party[4].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[5].stat_level_get(stat_hp_current) >= 1 and game.party[5].d20_query(Q_Prone) == 0):
						game.party[5].turn_towards(attachee)
						attachee.turn_towards(game.party[5])
						game.party[5].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[6].stat_level_get(stat_hp_current) >= 1 and game.party[6].d20_query(Q_Prone) == 0):
						game.party[6].turn_towards(attachee)
						attachee.turn_towards(game.party[6])
						game.party[6].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[7].stat_level_get(stat_hp_current) >= 1 and game.party[7].d20_query(Q_Prone) == 0):
						game.party[7].turn_towards(attachee)
						attachee.turn_towards(game.party[7])
						game.party[7].begin_dialog( attachee, 1 )
						game.new_sid = 0
						return SKIP_DEFAULT
				elif (find_npc_near(attachee,8032) != OBJ_HANDLE_NULL):
				## hedrack is alive and near
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						game.party[0].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[1].stat_level_get(stat_hp_current) >= 1 and game.party[1].d20_query(Q_Prone) == 0):
						game.party[1].turn_towards(attachee)
						game.party[1].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[2].stat_level_get(stat_hp_current) >= 1 and game.party[2].d20_query(Q_Prone) == 0):
						game.party[2].turn_towards(attachee)
						game.party[2].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[3].stat_level_get(stat_hp_current) >= 1 and game.party[3].d20_query(Q_Prone) == 0):
						game.party[3].turn_towards(attachee)
						game.party[3].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[4].stat_level_get(stat_hp_current) >= 1 and game.party[4].d20_query(Q_Prone) == 0):
						game.party[4].turn_towards(attachee)
						game.party[4].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[5].stat_level_get(stat_hp_current) >= 1 and game.party[5].d20_query(Q_Prone) == 0):
						game.party[5].turn_towards(attachee)
						game.party[5].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[6].stat_level_get(stat_hp_current) >= 1 and game.party[6].d20_query(Q_Prone) == 0):
						game.party[6].turn_towards(attachee)
						game.party[6].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[7].stat_level_get(stat_hp_current) >= 1 and game.party[7].d20_query(Q_Prone) == 0):
						game.party[7].turn_towards(attachee)
						game.party[7].begin_dialog( attachee, 100 )
						game.new_sid = 0
						return SKIP_DEFAULT
				else:
				## hedrack is dead or not near and party does not have golden skull
					if (game.party[0].stat_level_get(stat_hp_current) >= 1 and game.party[0].d20_query(Q_Prone) == 0):
						game.party[0].turn_towards(attachee)
						attachee.turn_towards(game.party[0])
						game.party[0].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[1].stat_level_get(stat_hp_current) >= 1 and game.party[1].d20_query(Q_Prone) == 0):
						game.party[1].turn_towards(attachee)
						attachee.turn_towards(game.party[1])
						game.party[1].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[2].stat_level_get(stat_hp_current) >= 1 and game.party[2].d20_query(Q_Prone) == 0):
						game.party[2].turn_towards(attachee)
						attachee.turn_towards(game.party[2])
						game.party[2].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[3].stat_level_get(stat_hp_current) >= 1 and game.party[3].d20_query(Q_Prone) == 0):
						game.party[3].turn_towards(attachee)
						attachee.turn_towards(game.party[3])
						game.party[3].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[4].stat_level_get(stat_hp_current) >= 1 and game.party[4].d20_query(Q_Prone) == 0):
						game.party[4].turn_towards(attachee)
						attachee.turn_towards(game.party[4])
						game.party[4].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[5].stat_level_get(stat_hp_current) >= 1 and game.party[5].d20_query(Q_Prone) == 0):
						game.party[5].turn_towards(attachee)
						attachee.turn_towards(game.party[5])
						game.party[5].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[6].stat_level_get(stat_hp_current) >= 1 and game.party[6].d20_query(Q_Prone) == 0):
						game.party[6].turn_towards(attachee)
						attachee.turn_towards(game.party[6])
						game.party[6].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
					elif (game.party[7].stat_level_get(stat_hp_current) >= 1 and game.party[7].d20_query(Q_Prone) == 0):
						game.party[7].turn_towards(attachee)
						attachee.turn_towards(game.party[7])
						game.party[7].begin_dialog( attachee, 130 )
						game.new_sid = 0
						return SKIP_DEFAULT
	return RUN_DEFAULT


def iuz_pc_persuade( iuz, pc, success, failure ):
	if (pc.saving_throw(10,D20_Save_Will,D20STD_F_NONE) == 0):
		pc.begin_dialog(iuz,failure)
	else:
		pc.begin_dialog(iuz,success)
	return SKIP_DEFAULT


def iuz_pc_charm( iuz, pc ):
	# auto dire charm the PC
	pc.dominate( iuz )
	if (game.party_pc_size() == 1):
		set_end_slides( iuz, pc )
		game.moviequeue_play_end_game()
	else:
		iuz.attack(pc)
	return SKIP_DEFAULT


def switch_to_hedrack( iuz, pc ):
	print "Iuz: Switching to Hedrack"
	hedrack = find_npc_near(iuz,8032)
	if (hedrack != OBJ_HANDLE_NULL):
		pc.begin_dialog(hedrack,200)
		hedrack.turn_towards(iuz)
		iuz.turn_towards(hedrack)
	else:
		pc.begin_dialog(iuz,120)
	return SKIP_DEFAULT


def switch_to_cuthbert( iuz, pc, line ):
	cuthbert = find_npc_near(iuz,8043)
	if (cuthbert != OBJ_HANDLE_NULL):
		pc.begin_dialog(cuthbert,line)
		cuthbert.turn_towards(iuz)
		iuz.turn_towards(cuthbert)
	else:
		iuz.object_flag_set(OF_OFF)
	return SKIP_DEFAULT


def find_ron( attachee, triggerer, line ):
	ron = find_npc_near(attachee,8730)
	cuthbert = find_npc_near(attachee,8043)
	if ((ron != OBJ_HANDLE_NULL) and (ron.distance_to(attachee) <= 12)):
		triggerer.begin_dialog(ron,line)
		ron.turn_towards(cuthbert)
	else:
		triggerer.begin_dialog(cuthbert,30)
		cuthbert.turn_towards(attachee)
		attachee.turn_towards(cuthbert)
	return SKIP_DEFAULT


def iuz_animate_troops( iuz, pc ):
	# raise or heal Hedrack and gargoyles
	game.particles( "sp-Unholy Blight", iuz )
	game.sound( 4016, 1 )
	if (find_npc_near( iuz, 8032 ) != OBJ_HANDLE_NULL):
		hedrack = find_npc_near( iuz, 8032 )
		if (hedrack.stat_level_get(stat_hp_current) <= -10):
			hedrack.resurrect( CRITTER_R_CUTHBERT_RESURRECT, 0 )
			for pc in game.party:
				hedrack.ai_shitlist_remove( pc )
				hedrack.reaction_set( pc, 50 )
			game.global_vars[780] = 0
			game.global_vars[781] = 0
		else:
			dice = dice_new("1d10+1000")
			hedrack.heal( OBJ_HANDLE_NULL, dice )
			hedrack.healsubdual( OBJ_HANDLE_NULL, dice )
			for pc in game.party:
				hedrack.ai_shitlist_remove( pc )
				hedrack.reaction_set( pc, 50 )
	if (find_npc_near( iuz, 8085 ) != OBJ_HANDLE_NULL):
		gargoyle_1 = find_npc_near( iuz, 8085 )
		if (gargoyle_1.stat_level_get(stat_hp_current) <= -10):
			gargoyle_1.resurrect( CRITTER_R_CUTHBERT_RESURRECT, 0 )
			for pc in game.party:
				gargoyle_1.ai_shitlist_remove( pc )
				gargoyle_1.reaction_set( pc, 50 )
		else:
			dice = dice_new("1d10+1000")
			gargoyle_1.heal( OBJ_HANDLE_NULL, dice )
			gargoyle_1.healsubdual( OBJ_HANDLE_NULL, dice )
			for pc in game.party:
				gargoyle_1.ai_shitlist_remove( pc )
				gargoyle_1.reaction_set( pc, 50 )
	if (find_npc_near( iuz, 8086 ) != OBJ_HANDLE_NULL):
		gargoyle_2 = find_npc_near( iuz, 8086 )
		if (gargoyle_2.stat_level_get(stat_hp_current) <= -10):
			gargoyle_2.resurrect( CRITTER_R_CUTHBERT_RESURRECT, 0 )
			for pc in game.party:
				gargoyle_2.ai_shitlist_remove( pc )
				gargoyle_2.reaction_set( pc, 50 )
		else:
			dice = dice_new("1d10+1000")
			gargoyle_2.heal( OBJ_HANDLE_NULL, dice )
			gargoyle_2.healsubdual( OBJ_HANDLE_NULL, dice )
			for pc in game.party:
				gargoyle_2.ai_shitlist_remove( pc )
				gargoyle_2.reaction_set( pc, 50 )
	if (find_npc_near( iuz, 8087 ) != OBJ_HANDLE_NULL):
		gargoyle_3 = find_npc_near( iuz, 8087 )
		if (gargoyle_3.stat_level_get(stat_hp_current) <= -10):
			gargoyle_3.resurrect( CRITTER_R_CUTHBERT_RESURRECT, 0 )
			for pc in game.party:
				gargoyle_3.ai_shitlist_remove( pc )
				gargoyle_3.reaction_set( pc, 50 )
		else:
			dice = dice_new("1d10+1000")
			gargoyle_3.heal( OBJ_HANDLE_NULL, dice )
			gargoyle_3.healsubdual( OBJ_HANDLE_NULL, dice )
			for pc in game.party:
				gargoyle_3.ai_shitlist_remove( pc )
				gargoyle_3.reaction_set( pc, 50 )
	if (find_npc_near( iuz, 8088 ) != OBJ_HANDLE_NULL):
		gargoyle_4 = find_npc_near( iuz, 8088 )
		if (gargoyle_4.stat_level_get(stat_hp_current) <= -10):
			gargoyle_4.resurrect( CRITTER_R_CUTHBERT_RESURRECT, 0 )
			for pc in game.party:
				gargoyle_4.ai_shitlist_remove( pc )
				gargoyle_4.reaction_set( pc, 50 )
		else:
			dice = dice_new("1d10+1000")
			gargoyle_4.heal( OBJ_HANDLE_NULL, dice )
			gargoyle_4.healsubdual( OBJ_HANDLE_NULL, dice )
			for pc in game.party:
				gargoyle_4.ai_shitlist_remove( pc )
				gargoyle_4.reaction_set( pc, 50 )
	# create zombies for dead allies by type and size or heal living
	if (find_npc_near( iuz, 8075 ) != OBJ_HANDLE_NULL):
		wizard_1 = find_npc_near( iuz, 8075 )
		if (wizard_1.stat_level_get(stat_hp_current) <= -10):
			wizard_zombie_1 = game.obj_create( 14820, wizard_1.location )
			wizard_zombie_1.rotation = wizard_1.rotation
			wizard_zombie_1.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", wizard_zombie_1 )
			wizard_1.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			wizard_1.heal( OBJ_HANDLE_NULL, dice )
			wizard_1.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8076 ) != OBJ_HANDLE_NULL):
		wizard_2 = find_npc_near( iuz, 8076 )
		if (wizard_2.stat_level_get(stat_hp_current) <= -10):
			wizard_zombie_2 = game.obj_create( 14820, wizard_2.location )
			wizard_zombie_2.rotation = wizard_2.rotation
			wizard_zombie_2.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", wizard_zombie_2 )
			wizard_2.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			wizard_2.heal( OBJ_HANDLE_NULL, dice )
			wizard_2.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8077 ) != OBJ_HANDLE_NULL):
		bugbear_archer_1 = find_npc_near( iuz, 8077 )
		if (bugbear_archer_1.stat_level_get(stat_hp_current) <= -10):
			bugbear_archer_zombie_1 = game.obj_create( 14821, bugbear_archer_1.location )
			bugbear_archer_zombie_1.rotation = bugbear_archer_1.rotation
			bugbear_archer_zombie_1.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_archer_zombie_1 )
			bugbear_archer_1.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_archer_1.heal( OBJ_HANDLE_NULL, dice )
			bugbear_archer_1.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8078 ) != OBJ_HANDLE_NULL):
		bugbear_archer_2 = find_npc_near( iuz, 8078 )
		if (bugbear_archer_2.stat_level_get(stat_hp_current) <= -10):
			bugbear_archer_zombie_2 = game.obj_create( 14821, bugbear_archer_2.location )
			bugbear_archer_zombie_2.rotation = bugbear_archer_2.rotation
			bugbear_archer_zombie_2.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_archer_zombie_2 )
			bugbear_archer_2.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_archer_2.heal( OBJ_HANDLE_NULL, dice )
			bugbear_archer_2.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8079 ) != OBJ_HANDLE_NULL):
		bugbear_fighter_1 = find_npc_near( iuz, 8079 )
		if (bugbear_fighter_1.stat_level_get(stat_hp_current) <= -10):
			bugbear_fighter_zombie_1 = game.obj_create( 14822, bugbear_fighter_1.location )
			bugbear_fighter_zombie_1.rotation = bugbear_fighter_1.rotation
			bugbear_fighter_zombie_1.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_fighter_zombie_1 )
			bugbear_fighter_1.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_fighter_1.heal( OBJ_HANDLE_NULL, dice )
			bugbear_fighter_1.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8080 ) != OBJ_HANDLE_NULL):
		bugbear_fighter_2 = find_npc_near( iuz, 8080 )
		if (bugbear_fighter_2.stat_level_get(stat_hp_current) <= -10):
			bugbear_fighter_zombie_2 = game.obj_create( 14822, bugbear_fighter_2.location )
			bugbear_fighter_zombie_2.rotation = bugbear_fighter_2.rotation
			bugbear_fighter_zombie_2.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_fighter_zombie_2 )
			bugbear_fighter_2.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_fighter_2.heal( OBJ_HANDLE_NULL, dice )
			bugbear_fighter_2.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8081 ) != OBJ_HANDLE_NULL):
		bugbear_fighter_3 = find_npc_near( iuz, 8081 )
		if (bugbear_fighter_3.stat_level_get(stat_hp_current) <= -10):
			bugbear_fighter_zombie_3 = game.obj_create( 14823, bugbear_fighter_3.location )
			bugbear_fighter_zombie_3.rotation = bugbear_fighter_3.rotation
			bugbear_fighter_zombie_3.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_fighter_zombie_3 )
			bugbear_fighter_3.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_fighter_3.heal( OBJ_HANDLE_NULL, dice )
			bugbear_fighter_3.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8082 ) != OBJ_HANDLE_NULL):
		bugbear_fighter_4 = find_npc_near( iuz, 8082 )
		if (bugbear_fighter_4.stat_level_get(stat_hp_current) <= -10):
			bugbear_fighter_zombie_4 = game.obj_create( 14823, bugbear_fighter_4.location )
			bugbear_fighter_zombie_4.rotation = bugbear_fighter_4.rotation
			bugbear_fighter_zombie_4.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", bugbear_fighter_zombie_4 )
			bugbear_fighter_4.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			bugbear_fighter_4.heal( OBJ_HANDLE_NULL, dice )
			bugbear_fighter_4.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8083 ) != OBJ_HANDLE_NULL):
		ettin_1 = find_npc_near( iuz, 8083 )
		if (ettin_1.stat_level_get(stat_hp_current) <= -10):
			ettin_zombie_1 = game.obj_create( 14824, ettin_1.location )
			ettin_zombie_1.rotation = ettin_1.rotation
			ettin_zombie_1.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", ettin_zombie_1 )
			ettin_1.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			ettin_1.heal( OBJ_HANDLE_NULL, dice )
			ettin_1.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8084 ) != OBJ_HANDLE_NULL):
		ettin_2 = find_npc_near( iuz, 8084 )
		if (ettin_2.stat_level_get(stat_hp_current) <= -10):
			ettin_zombie_2 = game.obj_create( 14824, ettin_2.location )
			ettin_zombie_2.rotation = ettin_2.rotation
			ettin_zombie_2.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", ettin_zombie_2 )
			ettin_2.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			ettin_2.heal( OBJ_HANDLE_NULL, dice )
			ettin_2.healsubdual( OBJ_HANDLE_NULL, dice )
	if (find_npc_near( iuz, 8089 ) != OBJ_HANDLE_NULL):
		hill_giant = find_npc_near( iuz, 8089 )
		if (hill_giant.stat_level_get(stat_hp_current) <= -10):
			hill_giant_zombie = game.obj_create( 14825, hill_giant.location )
			hill_giant_zombie.rotation = hill_giant.rotation
			hill_giant_zombie.condition_add_with_args("Prone",1,0)
			game.particles( "sp-Command Undead-Hit", hill_giant_zombie )
			hill_giant.object_flag_set(OF_OFF)
			game.global_vars[782] = game.global_vars[782] + 1
		else:
			dice = dice_new("1d10+1000")
			hill_giant.heal( OBJ_HANDLE_NULL, dice )
			hill_giant.healsubdual( OBJ_HANDLE_NULL, dice )
	game.timevent_add( make_hostile, (), 2000 )
	return SKIP_DEFAULT


def orientation( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	return RUN_DEFAULT


def unshit( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def make_hostile():
	for pc in game.party:
		if (find_npc_near( pc, 8032 ) != OBJ_HANDLE_NULL):
			hedrack = find_npc_near( pc, 8032 )
			hedrack.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8085 ) != OBJ_HANDLE_NULL):
			gargoyle_1 = find_npc_near( pc, 8085 )
			gargoyle_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8086 ) != OBJ_HANDLE_NULL):
			gargoyle_1 = find_npc_near( pc, 8086 )
			gargoyle_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8087 ) != OBJ_HANDLE_NULL):
			gargoyle_1 = find_npc_near( pc, 8087 )
			gargoyle_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8088 ) != OBJ_HANDLE_NULL):
			gargoyle_1 = find_npc_near( pc, 8088 )
			gargoyle_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8075 ) != OBJ_HANDLE_NULL):
			wizard_1 = find_npc_near( pc, 8075 )
			wizard_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8076 ) != OBJ_HANDLE_NULL):
			wizard_2 = find_npc_near( pc, 8076 )
			wizard_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8077 ) != OBJ_HANDLE_NULL):
			bugarcher_1 = find_npc_near( pc, 8077 )
			bugarcher_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8078 ) != OBJ_HANDLE_NULL):
			bugarcher_2 = find_npc_near( pc, 8078 )
			bugarcher_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8079 ) != OBJ_HANDLE_NULL):
			bugfighter_1 = find_npc_near( pc, 8079 )
			bugfighter_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8080 ) != OBJ_HANDLE_NULL):
			bugfighter_2 = find_npc_near( pc, 8080 )
			bugfighter_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8081 ) != OBJ_HANDLE_NULL):
			bugfighter_3 = find_npc_near( pc, 8081 )
			bugfighter_3.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8082 ) != OBJ_HANDLE_NULL):
			bugfighter_4 = find_npc_near( pc, 8082 )
			bugfighter_4.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8083 ) != OBJ_HANDLE_NULL):
			ettin_1 = find_npc_near( pc, 8083 )
			ettin_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8084 ) != OBJ_HANDLE_NULL):
			ettin_2 = find_npc_near( pc, 8084 )
			ettin_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 8089 ) != OBJ_HANDLE_NULL):
			giant = find_npc_near( pc, 8089 )
			giant.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14820 ) != OBJ_HANDLE_NULL):
			wizombie_1 = find_npc_near( pc, 14820 )
			wizombie_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14820 ) != OBJ_HANDLE_NULL):
			wizombie_2 = find_npc_near( pc, 14820 )
			wizombie_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14821 ) != OBJ_HANDLE_NULL):
			zombugarcher_1 = find_npc_near( pc, 14821 )
			zombugarcher_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14821 ) != OBJ_HANDLE_NULL):
			zombugarcher_2 = find_npc_near( pc, 14821 )
			zombugarcher_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14822 ) != OBJ_HANDLE_NULL):
			zombugfighter_1 = find_npc_near( pc, 14822 )
			zombugfighter_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14822 ) != OBJ_HANDLE_NULL):
			zombugfighter_2 = find_npc_near( pc, 14822 )
			zombugfighter_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14823 ) != OBJ_HANDLE_NULL):
			zombugfighter_3 = find_npc_near( pc, 14823 )
			zombugfighter_3.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14823 ) != OBJ_HANDLE_NULL):
			zombugfighter_4 = find_npc_near( pc, 14823 )
			zombugfighter_4.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14824 ) != OBJ_HANDLE_NULL):
			zombettin_1 = find_npc_near( pc, 14824 )
			zombettin_1.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14824 ) != OBJ_HANDLE_NULL):
			zombettin_2 = find_npc_near( pc, 14824 )
			zombettin_2.npc_flag_set(ONF_KOS)
		if (find_npc_near( pc, 14825 ) != OBJ_HANDLE_NULL):
			zombgiant = find_npc_near( pc, 14825 )
			zombgiant.npc_flag_set(ONF_KOS)
	return RUN_DEFAULT
from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	game.global_flags[70] = 1
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 350 )			## otis in party
	elif (game.global_vars[903] == 32 and attachee.map != 5051 and attachee.map != 5056):
		triggerer.begin_dialog( attachee, 560 )			## have attacked 3 or more farm animals with otis in party and not in nulb exterior or nulb smithy
	elif ((triggerer.item_find(2202) != OBJ_HANDLE_NULL) or (triggerer.item_find(3008) != OBJ_HANDLE_NULL)):
		triggerer.begin_dialog( attachee, 330 )			## you have otis chainmail and-or longsword in inventory
	elif ((game.quests[32].state == qs_completed) and (game.global_flags[74] == 0)):
		triggerer.begin_dialog( attachee, 270 )			## have completed bribery for profit quest and have not revealed otis secret
	elif (game.quests[31].state == qs_completed):
		triggerer.begin_dialog( attachee, 200 )			## have completed a second trip for otis quest
	elif (game.global_flags[73] == 1):
		triggerer.begin_dialog( attachee, 120 )			## otis has been to toee with you
	elif (game.quests[63].state == qs_accepted):
		triggerer.begin_dialog( attachee, 700 )			## bribery for justice quest accepted
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	print "Otis Enter Combat"
	if (triggerer.type == obj_t_pc):
		print 'Triggered!!!'
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			leader.follower_remove(attachee)
		elmo = find_npc_near( attachee, 8000)
		if (elmo != OBJ_HANDLE_NULL):
			attachee.float_line(380,triggerer)
			leader = elmo.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				leader.follower_remove(elmo)
			elmo.attack(triggerer)
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT		
		

def san_heartbeat( attachee, triggerer ):
	#print "Otis heartbeat"
	if (not game.combat_is_active()):
		# if (game.global_vars[903] >= 3): #removed buggy animal farm scripting
			# if (attachee != OBJ_HANDLE_NULL):
				# leader = attachee.leader_get()
				# if (leader != OBJ_HANDLE_NULL):
					# leader.follower_remove(attachee)
					# attachee.float_line(22000,triggerer)
		if (game.global_flags[362] == 0):
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (is_safe_to_talk(attachee,obj)):
					if ((obj.item_find(2202) != OBJ_HANDLE_NULL) or (obj.item_find(3008) != OBJ_HANDLE_NULL)):
						obj.begin_dialog(attachee,330)
						game.global_flags[362] = 1
						return RUN_DEFAULT
		if (game.global_flags[72] == 0):
			#print "elmo script"
			for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
				if (obj.name == 8000):
					#print "elmo found"
					delPc = GetDelegatePc(attachee, 10)
					if (delPc != OBJ_HANDLE_NULL):
						delPc.begin_dialog(attachee,400)	
						return RUN_DEFAULT
		if (game.global_flags[366] == 1):
			leader = attachee.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				attachee.turn_towards(leader)
				attachee.float_line(12023,leader)
				leader.follower_remove(attachee)
				attachee.attack(leader)
				game.global_flags[366] = 0
				return RUN_DEFAULT
		if (game.global_flags[367] == 1):
			leader = attachee.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				attachee.turn_towards(leader)
				attachee.float_line(10014,leader)
				leader.follower_remove(attachee)
				game.global_flags[367] = 0
				return RUN_DEFAULT
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	if (attachee.map == 5051):
		for chest in game.obj_list_vicinity(attachee.location,OLC_CONTAINER):
			if (chest.name == 1202):
				chest.item_transfer_to( attachee, 2202 )
				chest.item_transfer_to( attachee, 3008 )
				chest.item_transfer_to( attachee, 12038 )
				chest.item_transfer_to( attachee, 12040 )
				reg_warhammer = attachee.item_find(4077)
				if (reg_warhammer != OBJ_HANDLE_NULL):
					reg_warhammer.destroy()
				reg_chainmail = attachee.item_find(6019)
				if (reg_chainmail != OBJ_HANDLE_NULL):
					reg_chainmail.destroy()
				attachee.item_wield_best_all()
		mag_longsword = attachee.item_find(2202)
		if mag_longsword != OBJ_HANDLE_NULL:
			mag_longsword.item_flag_set(OIF_NO_TRANSFER)
		mag_chainmail = attachee.item_find(3008)
		if mag_chainmail != OBJ_HANDLE_NULL:
			mag_chainmail.item_flag_set(OIF_NO_TRANSFER)
		blu_sapph = attachee.item_find(12038)
		blu_sapph.item_flag_set(OIF_NO_TRANSFER)
		amber = attachee.item_find(12040)
		amber.item_flag_set(OIF_NO_TRANSFER)
	else:
		itemA = attachee.item_find(4077)
		if (itemA != OBJ_HANDLE_NULL):
			itemA.destroy()
		itemB = attachee.item_find(6019)
		if (itemB != OBJ_HANDLE_NULL):
			itemB.destroy()
		mag_sword = create_item_in_inventory( 4122, attachee )
		mag_sword.item_flag_set(OIF_NO_TRANSFER)
		mag_armor = create_item_in_inventory( 6102, attachee )
		mag_armor.item_flag_set(OIF_NO_TRANSFER)
		blu_sapp1 = create_item_in_inventory( 12038, attachee )
		blu_sapp1.item_flag_set(OIF_NO_TRANSFER)
		blu_sapp2 = create_item_in_inventory( 12038, attachee )
		blu_sapp2.item_flag_set(OIF_NO_TRANSFER)
		amber1 = create_item_in_inventory( 12040, attachee )
		amber1.item_flag_set(OIF_NO_TRANSFER)
		amber2 = create_item_in_inventory( 12040, attachee )
		amber2.item_flag_set(OIF_NO_TRANSFER)
		amber3 = create_item_in_inventory( 12040, attachee )
		amber3.item_flag_set(OIF_NO_TRANSFER)
		amber4 = create_item_in_inventory( 12040, attachee )
		amber4.item_flag_set(OIF_NO_TRANSFER)
		amber5 = create_item_in_inventory( 12040, attachee )
		amber5.item_flag_set(OIF_NO_TRANSFER)
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	for obj in triggerer.group_list():
		if (obj.name == 8021):
			triggerer.follower_remove(obj)
		if (obj.name == 8022):
			triggerer.follower_remove(obj)
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ((attachee.map == 5066) or (attachee.map == 5067) or (attachee.map == 5105) or (attachee.map == 5079) or (attachee.map == 5080)):
		game.global_flags[73] = 1
		if (game.quests[31].state == qs_accepted):
			game.quests[31].state = qs_completed
	elif ((attachee.map == 5062) or (attachee.map == 5112)):
		game.global_flags[73] = 1
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			if ((leader.stat_level_get(stat_alignment) == LAWFUL_EVIL) or (leader.stat_level_get(stat_alignment) == CHAOTIC_EVIL) or (leader.stat_level_get(stat_alignment) == NEUTRAL_EVIL)):
				percent = group_percent_hp(leader)
				if ((percent < 30) or (game.global_flags[74] == 1)):
					game.global_flags[366] = 1
	elif (attachee.map == 5051):
		if (((game.global_flags[73] == 1) and (game.quests[31].state == qs_unknown)) or (game.quests[31].state == qs_completed)):
			leader = attachee.leader_get()
			if (leader != OBJ_HANDLE_NULL):
				game.global_flags[367] = 1
	return RUN_DEFAULT


def make_elmo_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8000)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,410)
	return SKIP_DEFAULT


def make_saduj_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,14689)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	return SKIP_DEFAULT


def talk_to_screng( attachee, triggerer, line):
	npc = find_npc_near(attachee,8021)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,470)
	return SKIP_DEFAULT


def make_lila_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,14001)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,610)
	return SKIP_DEFAULT


def switch_to_thrommel( attachee, triggerer):
	npc = find_npc_near(attachee,8031)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,40)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,510)
	return SKIP_DEFAULT


def chain_it( attachee, triggerer ):
	itemA = attachee.item_find(2202)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.item_flag_unset(OIF_NO_TRANSFER)
	itemB = attachee.item_find(3008)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.item_flag_unset(OIF_NO_TRANSFER)
	return
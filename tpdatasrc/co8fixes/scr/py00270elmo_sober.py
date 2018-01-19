from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if (game.global_vars[900] == 32):
		triggerer.begin_dialog( attachee, 190 )			## have attacked 3 or more farm animals with elmo in party
	elif (attachee.has_met(triggerer)):
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			triggerer.begin_dialog( attachee, 90 )		## have met and elmo not in party
		triggerer.begin_dialog( attachee, 200 )			## elmo in party
	else:
		triggerer.begin_dialog( attachee, 1 )			## have not met
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	game.global_flags[934] = 1
	if (game.global_flags[236] == 0):
		game.global_vars[23] = game.global_vars[23] + 1
		if (game.global_vars[23] >= 2):
			game.party[0].reputation_add( 92 )
	else:
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[934] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		# if (game.global_vars[900] >= 3):
			# if (attachee != OBJ_HANDLE_NULL):
				# leader = attachee.leader_get()
				# if (leader != OBJ_HANDLE_NULL):
					# leader.follower_remove(attachee)
					# attachee.float_line(22000,triggerer)
		# else:
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				if (not attachee.has_met(obj)):
					obj.begin_dialog(attachee,1)
#						game.new_sid = 0
	return RUN_DEFAULT


def san_join(attachee, triggerer):
	game.global_flags[236] = 1
	print "elmo joins"
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	game.global_flags[236] = 0
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	randy1 = game.random_range(1,16)
	if ((attachee.map == 5052 or attachee.map == 5007) and randy1 >= 15):
		leader = attachee.leader_get()
		if (leader != OBJ_HANDLE_NULL):
			if (game.global_flags[934] == 0):
				attachee.float_line(12200,triggerer)
	return RUN_DEFAULT


def make_otis_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8014)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,320)
	return SKIP_DEFAULT


def make_lila_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,14001)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,320)
	return SKIP_DEFAULT


def make_Fruella_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,14037)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	return SKIP_DEFAULT


def make_saduj_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,14689)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	return SKIP_DEFAULT


def switch_to_thrommel( attachee, triggerer):
	npc = find_npc_near(attachee,8031)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,40)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,570)
	return SKIP_DEFAULT


def elmo_joins_first_time( attachee, triggerer, sober ):	#edited by darmagon for sober elmo
	if sober:
		loc = attachee.location
		rot = attachee.rotation		
		attachee.destroy()
		new_elmo = game.obj_create(14723, loc)
	else:
		new_elmo = attachee
	triggerer.money_adj(-20000)	
	rchain = create_item_in_inventory( 6049, new_elmo )
	rchain.item_flag_set(OIF_NO_TRANSFER)
	mshield = create_item_in_inventory( 6051, new_elmo)
	mshield.item_flag_set(OIF_NO_TRANSFER)
	maxe = create_item_in_inventory( 4098, new_elmo )
	maxe.item_flag_set(OIF_NO_TRANSFER)
	magd = attachee.item_find( 4058 )
	if magd != OBJ_HANDLE_NULL:
		magd.item_flag_set(OIF_NO_TRANSFER)
	new_elmo.item_wield_best_all()
	if sober:
		triggerer.begin_dialog(new_elmo, 70)
	return SKIP_DEFAULT


def equip_transfer( attachee, triggerer ):
	rchain = attachee.item_find( 6049 )
	if rchain != OBJ_HANDLE_NULL:
		rchain.item_flag_unset(OIF_NO_TRANSFER)
	mshield = attachee.item_find( 6051 )
	if mshield != OBJ_HANDLE_NULL:
		mshield.item_flag_unset(OIF_NO_TRANSFER)
	maxe = attachee.item_find( 4098 )
	if maxe != OBJ_HANDLE_NULL:
		maxe.item_flag_unset(OIF_NO_TRANSFER)
	return


def san_join(attachee, triggerer):
	print "elmo joins"
	return RUN_DEFAULT
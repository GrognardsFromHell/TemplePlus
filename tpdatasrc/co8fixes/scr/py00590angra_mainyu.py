from toee import *
from utilities import *
from scripts import *
from Co8 import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.turn_towards(attachee)
		triggerer.begin_dialog( attachee, 55 )
	else:
		triggerer.turn_towards(attachee)
		triggerer.begin_dialog( attachee, 1 )	
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_vars[980] == 3 and game.global_vars[981] == 3 and game.global_vars[982] == 3 and game.global_vars[983] == 3 and game.global_vars[984] == 3 and game.global_vars[985] == 3 and game.global_vars[986] == 3):	## turns on angra and co
		attachee.object_flag_unset(OF_OFF)
		if (not npc_get(attachee,4)):
			game.sound( 4183, 1 )
			npc_set(attachee,4)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.name == 8893):
		game.particles( "Orb-Summon-Glabrezu", attachee )
		game.global_flags[562] = 1
		if (game.global_flags[560] == 1 and game.global_flags[561] == 1):
			game.party[0].reputation_add( 62 )
		if (game.global_flags[564] == 0):
			game.party[0].reputation_add( 90 )
		attachee.object_flag_set(OF_OFF)
		spawn_phylactery()
	elif (attachee.name == 14949):
		game.particles( "hit-HOLY-medium", attachee )
		game.global_flags[564] = 1
		game.sound( 4184, 1 )
		if (game.global_flags[562] == 1):
			if (game.party[0].reputation_has(90) == 1):
				game.party[0].reputation_remove( 90 )
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	webbed = break_free( attachee, 3)
	if (obj_percent_hp(attachee) >= 51):
		if (game.global_vars[783] == 1):
			attachee.obj_set_int(obj_f_critter_strategy, 479)
		elif (game.global_vars[783] == 2):
			attachee.obj_set_int(obj_f_critter_strategy, 480)
		elif (game.global_vars[783] == 3):
			attachee.obj_set_int(obj_f_critter_strategy, 481)
		elif (game.global_vars[783] == 4):
			attachee.obj_set_int(obj_f_critter_strategy, 482)
		elif (game.global_vars[783] == 5):
			attachee.obj_set_int(obj_f_critter_strategy, 483)
		elif (game.global_vars[783] == 6):
			attachee.obj_set_int(obj_f_critter_strategy, 484)
		elif (game.global_vars[783] == 7):
			attachee.obj_set_int(obj_f_critter_strategy, 485)
		elif (game.global_vars[783] == 8):
			attachee.obj_set_int(obj_f_critter_strategy, 486)
		elif (game.global_vars[783] == 9):
			attachee.obj_set_int(obj_f_critter_strategy, 487)
		elif (game.global_vars[783] == 10):
			attachee.obj_set_int(obj_f_critter_strategy, 488)
		elif (game.global_vars[783] == 11):
			attachee.obj_set_int(obj_f_critter_strategy, 489)
		elif (game.global_vars[783] == 12):
			attachee.obj_set_int(obj_f_critter_strategy, 490)
		elif (game.global_vars[783] == 13):
			attachee.obj_set_int(obj_f_critter_strategy, 491)
		elif (game.global_vars[783] == 14):
			attachee.obj_set_int(obj_f_critter_strategy, 492)
		elif (game.global_vars[783] == 15):
			attachee.obj_set_int(obj_f_critter_strategy, 493)
		elif (game.global_vars[783] == 16):
			attachee.obj_set_int(obj_f_critter_strategy, 494)
		elif (game.global_vars[783] == 17):
			attachee.obj_set_int(obj_f_critter_strategy, 495)
		elif (game.global_vars[783] == 18):
			attachee.obj_set_int(obj_f_critter_strategy, 496)
		elif (game.global_vars[783] == 19):
			attachee.obj_set_int(obj_f_critter_strategy, 497)
		elif (game.global_vars[783] >= 20):
			attachee.obj_set_int(obj_f_critter_strategy, 478)
	elif (obj_percent_hp(attachee) <= 50):
		if (game.global_vars[783] == 19):
			attachee.obj_set_int(obj_f_critter_strategy, 497)
		elif (game.global_vars[783] == 20):
			attachee.obj_set_int(obj_f_critter_strategy, 478)
		elif (game.global_vars[783] != 20):
			attachee.obj_set_int(obj_f_critter_strategy, 486)
	game.particles( "Trap-Spores", attachee )
	fx = game.random_range(1,3)
	if (fx == 1):
		game.sound( 4167, 1 )
	elif (fx == 2):
		game.sound( 4168, 1 )
	elif (fx == 3):
		game.sound( 4169, 1 )
	return RUN_DEFAULT


def san_end_combat( attachee, triggerer ):
	if (obj_percent_hp(attachee) >= 51):
		if (game.global_vars[783] == 0):
			game.global_vars[783] = 1
		elif (game.global_vars[783] == 1):
			game.global_vars[783] = 2
		elif (game.global_vars[783] == 2):
			game.global_vars[783] = 3
		elif (game.global_vars[783] == 3):
			game.global_vars[783] = 4
		elif (game.global_vars[783] == 4):
			game.global_vars[783] = 5
		elif (game.global_vars[783] == 5):
			game.global_vars[783] = 6
		elif (game.global_vars[783] == 6):
			game.global_vars[783] = 7
		elif (game.global_vars[783] == 7):
			game.global_vars[783] = 8
		elif (game.global_vars[783] == 8):
			game.global_vars[783] = 9
		elif (game.global_vars[783] == 9):
			game.global_vars[783] = 10
		elif (game.global_vars[783] == 10):
			game.global_vars[783] = 11
		elif (game.global_vars[783] == 11):
			game.global_vars[783] = 12
		elif (game.global_vars[783] == 12):
			game.global_vars[783] = 13
		elif (game.global_vars[783] == 13):
			game.global_vars[783] = 14
		elif (game.global_vars[783] == 14):
			game.global_vars[783] = 15
		elif (game.global_vars[783] == 15):
			game.global_vars[783] = 16
		elif (game.global_vars[783] == 16):
			game.global_vars[783] = 17
		elif (game.global_vars[783] == 17):
			game.global_vars[783] = 18
		elif (game.global_vars[783] == 18):
			game.global_vars[783] = 19
		elif (game.global_vars[783] == 19):
			game.global_vars[783] = 20
	elif (obj_percent_hp(attachee) <= 50):
		if (game.global_vars[783] == 19):
			game.global_vars[783] = 20
		elif (game.global_vars[783] != 20):
			game.global_vars[783] = 19
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.name == 8893):
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			if (not game.combat_is_active()):
				if (game.global_vars[980] == 3 and game.global_vars[981] == 3 and game.global_vars[982] == 3 and game.global_vars[983] == 3 and game.global_vars[984] == 3 and game.global_vars[985] == 3 and game.global_vars[986] == 3):	## turns on angra
					attachee.object_flag_unset(OF_OFF)
					if (not npc_get(attachee,4)):
						game.sound( 4183, 1 )
						npc_set(attachee,4)
				closest_jones = party_closest(attachee)
				if (attachee.distance_to(closest_jones) <= 100 ):
					if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
						if (game.global_vars[980] == 3 and game.global_vars[981] == 3 and game.global_vars[982] == 3 and game.global_vars[983] == 3 and game.global_vars[984] == 3 and game.global_vars[985] == 3 and game.global_vars[986] == 3 and game.global_flags[563] == 0):
							game.global_vars[973] = game.global_vars[973] + 1
							if (game.global_vars[973] == 10 or game.global_vars[973] == 20 or game.global_vars[973] == 30 or game.global_vars[973] == 40):
								game.particles( "Trap-Spores", attachee )
								fx = game.random_range(1,4)
								if (fx == 1):
									game.sound( 4167, 1 )
								elif (fx == 2):
									game.sound( 4168, 1 )
								elif (fx == 3):
									game.sound( 4169, 1 )
							if (game.global_vars[973] == 40):
								game.global_vars[973] = 0
				if (game.global_flags[984] == 0):
					for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
						if (is_35_and_under(attachee, obj)):
							attachee.turn_towards(game.party[0])
							game.party[0].begin_dialog( attachee, 1 )
							game.global_flags[984] = 1
				if (game.global_flags[563] == 1 and not npc_get(attachee,2)):
					game.timevent_add( angra_exit, ( attachee, triggerer ), 200 )
					npc_set(attachee,2)
		if (npc_get(attachee,3)):
			attachee.object_flag_set(OF_OFF)
	elif (attachee.name == 8614 or attachee.name == 8615 or attachee.name == 8616 or attachee.name == 8617 or attachee.name == 8618 or attachee.name == 8619 or attachee.name == 8620 or attachee.name == 8621):
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			if (not game.combat_is_active()):
				if (game.global_vars[980] == 3 and game.global_vars[981] == 3 and game.global_vars[982] == 3 and game.global_vars[983] == 3 and game.global_vars[984] == 3 and game.global_vars[985] == 3 and game.global_vars[986] == 3):	## turns on angra co
					attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def is_35_and_under(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 35):
			return 1
	return 0


def angra_exit( attachee, triggerer ):
	attachee.npc_flag_unset(ONF_WAYPOINTS_DAY)
	attachee.npc_flag_unset(ONF_WAYPOINTS_NIGHT)
	attachee.standpoint_set( STANDPOINT_NIGHT, 434 )
	attachee.standpoint_set( STANDPOINT_DAY, 434 )
#	attachee.standpoint_set( STANDPOINT_SCOUT, 434 )
	attachee.runoff( location_from_axis( 481, 477 ) )
	bug1 = find_npc_near(attachee,8614)
	bug1.runoff( location_from_axis( 481, 477 ) )
	bug2 = find_npc_near(attachee,8615)
	bug2.runoff( location_from_axis( 481, 477 ) )
	ass1 = find_npc_near(attachee,8616)
	ass1.runoff( location_from_axis( 481, 477 ) )
	tra1 = find_npc_near(attachee,8617)
	tra1.runoff( location_from_axis( 481, 477 ) )
	ogr1 = find_npc_near(attachee,8618)
	ogr1.runoff( location_from_axis( 481, 477 ) )
	ett1 = find_npc_near(attachee,8619)
	ett1.runoff( location_from_axis( 481, 477 ) )
	sto1 = find_npc_near(attachee,8620)
	sto1.runoff( location_from_axis( 481, 477 ) )
	hil1 = find_npc_near(attachee,8621)
	hil1.runoff( location_from_axis( 481, 477 ) )
	game.timevent_add( angra_off, ( attachee, triggerer ), 8000 )
	game.timevent_add( bug1_off, ( bug1, triggerer ), 8000 )
	game.timevent_add( bug2_off, ( bug2, triggerer ), 8000 )
	game.timevent_add( ass1_off, ( ass1, triggerer ), 8000 )
	game.timevent_add( tra1_off, ( tra1, triggerer ), 8000 )
	game.timevent_add( ogr1_off, ( ogr1, triggerer ), 8000 )
	game.timevent_add( ett1_off, ( ett1, triggerer ), 8000 )
	game.timevent_add( sto1_off, ( sto1, triggerer ), 8000 )
	game.timevent_add( hil1_off, ( hil1, triggerer ), 8000 )
	return RUN_DEFAULT


def angra_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	npc_set(attachee,3)
	return RUN_DEFAULT


def bug1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def bug2_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def ass1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def tra1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def ogr1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def ett1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def sto1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def hil1_off( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def increment_rep( attachee, triggerer ):
	if (game.party[0].reputation_has(81) == 1):
		game.party[0].reputation_add(82)
		game.party[0].reputation_remove(81)
	elif (game.party[0].reputation_has(82) == 1):
		game.party[0].reputation_add(83)
		game.party[0].reputation_remove(82)
	elif (game.party[0].reputation_has(83) == 1):
		game.party[0].reputation_add(84)
		game.party[0].reputation_remove(83)
	elif (game.party[0].reputation_has(84) == 1):
		game.party[0].reputation_add(85)
		game.party[0].reputation_remove(84)
	elif (game.party[0].reputation_has(85) == 1):
		game.party[0].reputation_add(86)
		game.party[0].reputation_remove(85)
	elif (game.party[0].reputation_has(86) == 1):
		game.party[0].reputation_add(87)
		game.party[0].reputation_remove(86)
	elif (game.party[0].reputation_has(87) == 1):
		game.party[0].reputation_add(88)
		game.party[0].reputation_remove(87)
	elif (game.party[0].reputation_has(88) == 1):
		game.party[0].reputation_add(89)
		game.party[0].reputation_remove(88)
	else:
		game.party[0].reputation_add(81)
	return RUN_DEFAULT


def spawn_phylactery():
	loc = game.random_range(1,12)
	if (loc == 1):
		phyl_1 = game.obj_create( 14949, location_from_axis (435L, 368L) )
	elif (loc == 2):
		phyl_2 = game.obj_create( 14949, location_from_axis (352L, 470L) )
	elif (loc == 3):
		phyl_3 = game.obj_create( 14949, location_from_axis (400L, 456L) )
	elif (loc == 4):
		phyl_4 = game.obj_create( 14949, location_from_axis (543L, 377L) )
	elif (loc == 5):
		phyl_5 = game.obj_create( 14949, location_from_axis (429L, 509L) )
	elif (loc == 6):
		phyl_6 = game.obj_create( 14949, location_from_axis (384L, 525L) )
	elif (loc == 7):
		phyl_7 = game.obj_create( 14949, location_from_axis (383L, 553L) )
	elif (loc == 8):
		phyl_8 = game.obj_create( 14949, location_from_axis (605L, 438L) )
	elif (loc == 9):
		phyl_9 = game.obj_create( 14949, location_from_axis (527L, 515L) )
	elif (loc == 10):
		phyl_10 = game.obj_create( 14949, location_from_axis (478L, 579L) )
	elif (loc == 11):
		phyl_11 = game.obj_create( 14949, location_from_axis (457L, 632L) )
	elif (loc == 12):
		phyl_12 = game.obj_create( 14949, location_from_axis (536L, 565L) )
	return
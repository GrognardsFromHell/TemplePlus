from toee import *
from utilities import *
from toee import anyone
from py00439script_daemon import *
import _include
from co8Util.TimedEvent import *
from combat_standard_routines import *
from py00439script_daemon import get_f, set_f, get_v, set_v, tpsts, record_time_stamp


def san_dialog( attachee, triggerer ):
	if game.global_vars[923] == 0:
		tempp = 0
		for p in range(0, 12):
			tempp += game.random_range(0, 8)
		tempp -= 24
		if tempp < 5:
			tempp = 5
		game.global_vars[923] = tempp
	elif tpsts('s_ranths_bandits_1', 0) == 0:
		record_time_stamp('s_ranths_bandits_1')
	attachee.turn_towards(triggerer)
	if (game.quests[78].state == qs_completed and game.quests[107].state == qs_unknown and game.quests[112].state == qs_mentioned):
		triggerer.begin_dialog( attachee, 430 )
	if (game.quests[74].state == qs_completed and game.quests[78].state == qs_unknown and game.quests[111].state == qs_mentioned):
		triggerer.begin_dialog( attachee, 450 )
	elif (game.global_vars[993] == 7):
		triggerer.begin_dialog( attachee, 630 )
	elif (game.global_vars[993] == 9):
		triggerer.begin_dialog( attachee, 710 )
	elif (attachee.map == 5156):
		triggerer.begin_dialog( attachee, 910 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[992] == 1):
		attachee.object_flag_set(OF_OFF)
	elif (attachee.map == 5156 and game.global_vars[704] == 3 and is_daytime() == 1 and game.quests[76].state != qs_accepted):
		attachee.object_flag_unset(OF_OFF)
	elif (attachee.map == 5170 and game.global_vars[979] == 2):
		if (is_daytime() == 1):
			attachee.object_flag_unset(OF_OFF)
		elif (is_daytime() == 0):
			attachee.object_flag_set(OF_OFF)
	elif (attachee.map == 5135 and game.global_vars[979] == 2):
		if (is_daytime() == 1):
			attachee.object_flag_set(OF_OFF)
		elif (is_daytime() == 0):
			attachee.object_flag_unset(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	for pc in game.party:
		pc.condition_add('fallen_paladin')
	if (attachee.map == 5170 or attachee.map == 5135):
		game.global_flags[992] = 1
		game.global_flags[935] = 1
		game.party[0].reputation_add( 44 )
	elif (attachee.map == 5156):
		if (game.global_flags[940] == 1):
			game.global_flags[935] = 1
			game.party[0].reputation_add( 44 )
		game.global_flags[992] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (attachee.name == 8703):
		if (attachee.map == 5156):
			attachee.float_line( 5000,triggerer )
		if (attachee.map == 5170):
			samson = game.obj_create(14660, location_from_axis (501L, 484L))
			samson.turn_towards(triggerer)
			samson.attack(game.party[0])
			goliath = game.obj_create(14661, location_from_axis (498L, 484L))
			goliath.turn_towards(triggerer)
			goliath.attack(game.party[0])
			bathsheba = game.obj_create(14659, location_from_axis (495L, 484L))
			bathsheba.turn_towards(triggerer)
			bathsheba.float_line(1000,triggerer)
			bathsheba.attack(game.party[0])
		if (attachee.map == 5135 and attachee.name == 8703):
			samson = game.obj_create(14660, location_from_axis (494L, 488L))
			samson.turn_towards(triggerer)
			samson.attack(game.party[0])
			goliath = game.obj_create(14661, location_from_axis (494L, 491L))
			goliath.turn_towards(triggerer)
			goliath.attack(game.party[0])
			bathsheba = game.obj_create(14659, location_from_axis (481L, 496L))
			bathsheba.turn_towards(triggerer)
			bathsheba.float_line(1000,triggerer)
			bathsheba.attack(game.party[0])
	ProtectTheInnocent(attachee, triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	game.counters[0] = game.counters[0] + 1
	if (game.counters[0] == 1):
		attachee.float_line(1000,triggerer)
		return SKIP_DEFAULT
	elif (game.counters[0] == 2):
		overseers_show_up( attachee, triggerer )
		game.global_vars[704] = 4
	elif (game.counters[0] == 3):
		attachee.float_line(2000,triggerer)
		return SKIP_DEFAULT
	elif (game.counters[0] == 4):
		guards_show_up( attachee, triggerer )
		game.global_vars[704] = 5
	elif (game.counters[0] == 5):
		attachee.float_line(4000,triggerer)
		return SKIP_DEFAULT
	elif (game.counters[0] == 6):
		guardian_show_up( attachee, triggerer )
		game.global_vars[704] = 6
	elif (game.counters[0] == 7):
		attachee.float_line(3000,triggerer)
		return SKIP_DEFAULT
	elif (game.counters[0] == 8):
		mages_show_up( attachee, triggerer )
		game.global_vars[704] = 7
	elif (game.counters[0] == 9):
		game.global_vars[704] = 8
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.party[0].reputation_has(34) == 1):
		return RUN_DEFAULT
	elif (game.global_flags[992] == 0):
		return SKIP_DEFAULT
	return RUN_DEFAULT


def distribute_verbobonc_uniform(npc,pc):
	for obj in pc.group_list():
		create_item_in_inventory( 6498, obj )
		create_item_in_inventory( 6269, obj )
	return RUN_DEFAULT


def overseers_show_up( attachee, triggerer ):
	samson = game.obj_create(14660, location_from_axis (482L, 494L))
	samson.turn_towards(triggerer)
	samson.float_line(1000,triggerer)
	goliath = game.obj_create(14661, location_from_axis (484L, 495L))
	goliath.turn_towards(triggerer)
	samson.attack(game.party[0])
	goliath.attack(game.party[0])
	return RUN_DEFAULT


def guards_show_up( attachee, triggerer ):
	guard1 = game.obj_create(14644, location_from_axis (481L, 493L))
	guard1.turn_towards(triggerer)
	guard1.float_line(1000,triggerer)
	guard2 = game.obj_create(14644, location_from_axis (483L, 495L))
	guard2.turn_towards(triggerer)
	guard3 = game.obj_create(14644, location_from_axis (479L, 493L))
	guard3.turn_towards(triggerer)
	guard4 = game.obj_create(14644, location_from_axis (481L, 495L))
	guard4.turn_towards(triggerer)
	guard1.attack(game.party[0])
	guard2.attack(game.party[0])
	guard3.attack(game.party[0])
	guard4.attack(game.party[0])
	return RUN_DEFAULT


def guardian_show_up( attachee, triggerer ):
	bathsheba = game.obj_create(14659, location_from_axis (484L, 494L))
	bathsheba.turn_towards(triggerer)
	bathsheba.float_line(2000,triggerer)
	bathsheba.attack(game.party[0])
	return RUN_DEFAULT


def mages_show_up( attachee, triggerer ):
	mage1 = game.obj_create(14658, attachee.location-4)
	game.particles( "sp-Teleport", mage1 )
	mage1.turn_towards(triggerer)
	mage1.float_line(1000,triggerer)
	mage2 = game.obj_create(14658, attachee.location-4)
	game.particles( "sp-Teleport", mage2 )
	mage2.turn_towards(triggerer)
	game.sound( 4035, 1 )
	for obj in game.obj_list_vicinity(mage1.location,OLC_PC):
		mage1.attack(obj)
	for obj in game.obj_list_vicinity(mage2.location,OLC_PC):
		mage2.attack(obj)
	return RUN_DEFAULT


def ditch_captains( attachee, triggerer ):
	abiram = find_npc_near(attachee,8706)
	abiram.runoff(attachee.location-3)
	absalom = find_npc_near(attachee,8707)
	absalom.runoff(attachee.location-3)
	achan = find_npc_near(attachee,8708)
	achan.runoff(attachee.location-3)
	return


def switch_to_captain( attachee, triggerer, line):
	abiram = find_npc_near(attachee,8706)
	absalom = find_npc_near(attachee,8707)
	achan = find_npc_near(attachee,8708)
	if (abiram != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(abiram, line)
	if (absalom != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(absalom, line)
	if (achan != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(achan, line)
	return SKIP_DEFAULT


def schedule_bandits_1( attachee, triggerer ):
	tempp = game.global_vars[923]
	if game.global_vars[923] == 0:
		for p in range(0, 12):
			tempp += game.random_range(0, 8)
		tempp -= 24
		if tempp < 5:
			tempp = 5
		# approximate a gaussian distribution by adding together 12 uniformly distributed random variables
		# average result will be 24 days, standard deviation will be 8 days
		# it is then truncated at 5 days minimum (feel free to change) (roughly 1% of results might reach 5 or lower otherwise, even negative is possible though rare)
		game.global_vars[923] = tempp
	game.timevent_add( set_bandits, (), tempp * 24 * 60 * 60 * 1000 )
	record_time_stamp('s_ranths_bandits_1')
	return RUN_DEFAULT




def set_bandits():
	game.encounter_queue.append(3434)
	set_f('s_ranths_bandits_scheduled')
	return RUN_DEFAULT


def slavers_movie_setup( attachee, triggerer ):
	set_slavers_slides()
	return


def set_slavers_slides():
	game.moviequeue_add(601)
	game.moviequeue_add(602)
	game.moviequeue_add(603)
	game.moviequeue_add(604)
	game.moviequeue_add(605)
	game.moviequeue_add(606)
	game.moviequeue_add(607)
	game.moviequeue_add(608)
	game.moviequeue_add(609)
	game.moviequeue_play()
	return RUN_DEFAULT
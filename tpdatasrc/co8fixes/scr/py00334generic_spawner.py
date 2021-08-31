from toee import *
from utilities import *
from __main__ import game
from Co8 import *
from py00439script_daemon import *
import _include
from co8Util.TimedEvent import *
from py00439script_daemon import get_f, set_f, get_v, set_v, tpsts, record_time_stamp
from combat_standard_routines import *
from co8Util.PersistentData import *
from co8Util.ObjHandling import *


# Contained in this script:
#  -Temple exterior: Flags Burnt Farm, Ruined Temple House, Temple Tower visited
#  -Temple lvl2: Fix for giving Countess Tillahi instructions if you've been taken blindfolded into the Temple


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5001 or attachee.map == 5002 or attachee.map == 5051 or attachee.map == 5062 or attachee.map == 5068 or attachee.map == 5069 or attachee.map == 5091 or attachee.map == 5112 or attachee.map == 5094 or attachee.map == 5113 or attachee.map == 5108 or attachee.map == 5093 or attachee.map == 5095 or attachee.map == 5111 or attachee.map == 5121 or attachee.map == 5132):
	# All Worldmap Locations
	# 5001 = hommlet exterior - game area 1
	# 5002 = moathouse exterior - game area 2
	# 5051 = nulb exterior - game area 3
	# 5062 = temple entrance - game area 4
	# 5068 = imeryds run - game area 6
	# 5069 = decklo grove - game area 10
	# 5091 = moathouse exit tunnel - game area 8
	# 5112 = temple deserted farm - game area 11
	# 5094 = emridy meadows - game area 5
	# 5113 = ogre cave exterior - game area 12
	# 5108 = quarry exterior - game area 16
	# 5093 = welkwood bog exterior - game area 7
	# 5095 = hickory branch exterior - game area 9
	# 5121 = verbobonc exterior - game area 14
	# 5132 = verbobonc cave exit - game area 15
		if (attachee.map == 5001):
		# Hommlet Exterior
			hommlet_movie_check()
		if (attachee.map == 5002):
		# Moathouse Exterior
			moathouse_respawn_check()
			moathouse_movie_check()
		if (attachee.map == 5051):
		# Nulb Exterior
			nulb_movie_check()
		if (attachee.map == 5062):
		# Temple Entrance
			temple_movie_check()
		if (attachee.map == 5093):
		# Welkwood Exterior
			welkwood_movie_check()
		if (attachee.map == 5094):
		# Emridy Meadows
			emridy_movie_check()
		if (attachee.map == 5095):
		# Hickory Branch Exterior
			hickory_movie_check()
		if (attachee.map == 5121):
		# Verbobonc Exterior
			verbo_tax( attachee, triggerer )
			verbobonc_movie_check()
		if (attachee.map != 5001):
		# not Hommlet Exterior
			hextor_wins()
		if (attachee.map != 5002):
		# not Moathouse Exterior
			party_death_by_poison()
			if (attachee.map != 5091):
			# not Moathouse Cave Exit
				big_3_check()
		if (attachee.map != 5095):
		# not Hickory Branch
			hickory_branch_check()

		# Sitra stuff below - next, check if there's a PC possessed by the new_map script, and if not - assign it to one
		scriptbearer_found = 0
		for scriptbearer in game.party:
			if scriptbearer.scripts[38] == 439:
				scriptbearer_found = 1
			if scriptbearer.scripts[38] != 439 and scriptbearer.stat_level_get( stat_hp_current ) > -10 and scriptbearer_found == 0 and scriptbearer.type == obj_t_pc:
				scriptbearer.scripts[12] = 439 # san_dying
				scriptbearer.scripts[14] = 439 # san_exit_combat
				scriptbearer.scripts[38] = 439 # san_new_map
				scriptbearer_found = 1
	elif (attachee.map == 5067):
	# Temple Level 2
		bin_thr_dun_tht()
	elif (attachee.map == 5064 or attachee.map == 5105):
	# Temple Int and Level 3 Lower
		give_directions()
	elif (attachee.map == 5107):
	# shop map
		money_script()
		shopmap_movie_check()
		Co8PersistentData.clearData()   # clears persistent data
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.map == 5112):
	# Temple Deserted Farm
		game.areas[11] = 1
	elif (attachee.map == 5091):
	# Moathouse Cave Exit
		game.areas[8] = 1
	elif (attachee.map == 5132):
	# Verbobonc Cave Exit
		game.areas[15] = 1
	return SKIP_DEFAULT


def money_script():
	# money script by Agetian
	# strip all money
	game.global_flags[500] = 1
	# added by Gaear - denotes you are playing NC
	if (game.leader.money_get() >= 50000):
		game.party[0].money_adj(-game.party[0].money_get())
		#add 200-300 gold pocket money
		game.party[0].money_adj(game.random_range(20000, 30000))
	for pc in game.party:
		lsw = pc.item_find_by_proto(4036)
		scm = pc.item_find_by_proto(4045)
		# get rid of an extra longsword (4036) and scimitar(4045)
		for pp in range(0, 24):
			invItem = pc.inventory_item(pp)
			if (invItem.proto == 4036 and invItem != lsw):
				invItem.destroy()
			if (invItem.proto == 4045 and invItem != scm):
				invItem.destroy()
		# give money per PHB (for each class)
		if (pc.stat_level_get(stat_level_barbarian) > 0):
			pc.money_adj(game.random_range(4000, 16000))
		if (pc.stat_level_get(stat_level_bard) > 0):
			pc.money_adj(game.random_range(4000, 16000))
		if (pc.stat_level_get(stat_level_cleric) > 0):
			pc.money_adj(game.random_range(5000, 20000))
		if (pc.stat_level_get(stat_level_druid) > 0):
			pc.money_adj(game.random_range(2000, 8000))
		if (pc.stat_level_get(stat_level_fighter) > 0):
			pc.money_adj(game.random_range(6000, 24000))
		if (pc.stat_level_get(stat_level_monk) > 0):
			pc.money_adj(game.random_range(5000, 20000))
		if (pc.stat_level_get(stat_level_paladin) > 0):
			pc.money_adj(game.random_range(6000, 24000))
		if (pc.stat_level_get(stat_level_ranger) > 0):
			pc.money_adj(game.random_range(6000, 24000))
		if (pc.stat_level_get(stat_level_rogue) > 0):
			pc.money_adj(game.random_range(5000, 20000))
		if (pc.stat_level_get(stat_level_sorcerer) > 0):
			pc.money_adj(game.random_range(3000, 12000))
		if (pc.stat_level_get(stat_level_wizard) > 0):
			pc.money_adj(game.random_range(3000, 12000))
	return


def bin_thr_dun_tht():
	#bin_thr_dun_tht script by rufnredde - Function: Sets ggf 286 if you have been to level 2 and resets ggf 287 enter temple blindfolded if you have entered/exited normally ggv 55
	game.global_flags[286] = 1
	if (game.global_flags[287] == 1 and game.global_vars[55] >= 2 and game.global_flags[286] == 1):
		game.global_flags[287] = 0
	return


def give_directions():
	#give_directions script by rufnredde - Function: Checks to make sure you have entered the temple ggv 55 other than blindfolded ggf 287 and have been to level 2 ggf 286 allows you to give directions out of the temple
	if (game.global_flags[287] == 0 and game.global_vars[55] <= 1 and game.global_flags[286] == 0):
		game.global_vars[55] = 1
	if (game.global_flags[287] == 0 and game.global_vars[55] <=1 and game.global_flags[286] == 1):
		game.global_vars[55] = game.global_vars[55] + 1
	if (game.global_flags[287] == 1 and game.global_vars[55] <= 1):
		game.global_vars[55] = game.global_vars[55] + 1
	if (game.global_flags[287] == 1) and ((game.quests[53].state == qs_completed or game.quests[53].state == qs_accepted) or (game.global_flags[996] == 1 or game.global_flags[998] == 1)):
		game.global_vars[55] = game.global_vars[55] + 2
	if (game.global_flags[287] == 1 and game.global_vars[55] >= 2):
		game.global_flags[287] = 0
	return


def big_3_check():
	if (game.global_flags[37] == 1) and (game.global_vars[765] == 0):
		game.global_flags[283] = 1
	return


def hickory_branch_check():
	if (game.global_vars[982] == 1 or game.global_vars[983] == 1 or game.global_vars[984] == 1 or game.global_vars[985] == 1):
		if (game.global_vars[980] != 3):
			game.global_vars[980] = 2
		if (game.global_vars[981] != 3):
			game.global_vars[981] = 2
		if (game.global_vars[982] != 3):
			game.global_vars[982] = 2
		if (game.global_vars[983] != 3):
			game.global_vars[983] = 2
		if (game.global_vars[984] != 3):
			game.global_vars[984] = 2
	return


def moathouse_respawn_check():
	if (game.quests[106].state == qs_mentioned):
		game.quests[106].state = qs_completed
		game.quests[95].state = qs_mentioned
		game.sound( 4166, 1 )
	return


def shopmap_movie_check():
	if (game.global_flags[602] == 0):
		set_shopmap_slides()
		game.global_flags[602] = 1
	return


def set_shopmap_slides():
	game.moviequeue_add(400)
	game.moviequeue_add(401)
	game.moviequeue_play()
	return RUN_DEFAULT


def hommlet_movie_check():
	if (game.global_flags[603] == 0):
		set_hommlet_slides()
		game.global_flags[603] = 1
	return


def set_hommlet_slides():
	game.moviequeue_add(402)
	game.moviequeue_add(403)
	game.moviequeue_play()
	return RUN_DEFAULT


def welkwood_movie_check():
	if (game.global_flags[605] == 0):
		set_welkwood_slides()
		game.global_flags[605] = 1
	return


def set_welkwood_slides():
	game.moviequeue_add(404)
	game.moviequeue_add(405)
	game.moviequeue_play()
	return RUN_DEFAULT


def emridy_movie_check():
	if (game.global_flags[607] == 0):
		set_emridy_slides()
		game.global_flags[607] = 1
	return


def set_emridy_slides():
	game.moviequeue_add(406)
	game.moviequeue_add(407)
	game.moviequeue_play()
	return RUN_DEFAULT


def moathouse_movie_check():
	if (game.global_flags[604] == 0):
		set_moathouse_slides()
		game.global_flags[604] = 1
	return


def set_moathouse_slides():
	game.moviequeue_add(408)
	game.moviequeue_add(409)
	game.moviequeue_play()
	return RUN_DEFAULT


def nulb_movie_check():
	if (game.global_flags[609] == 0):
		set_nulb_slides()
		game.global_flags[609] = 1
	return


def set_nulb_slides():
	game.moviequeue_add(410)
	game.moviequeue_add(411)
	game.moviequeue_play()
	return RUN_DEFAULT


def temple_movie_check():
	if (game.global_flags[610] == 0):
		set_temple_slides()
		game.global_flags[610] = 1
	return


def set_temple_slides():
	game.moviequeue_add(412)
	game.moviequeue_add(413)
	game.moviequeue_add(414)
	game.moviequeue_add(415)
	game.moviequeue_play()
	return RUN_DEFAULT


def verbobonc_movie_check():
	if (game.global_flags[606] == 0):
		set_verbobonc_slides()
		game.global_flags[606] = 1
	return


def set_verbobonc_slides():
	game.moviequeue_add(416)
	game.moviequeue_add(417)
	game.moviequeue_play()
	return RUN_DEFAULT


def hickory_movie_check():
	if (game.global_flags[608] == 0):
		set_hickory_slides()
		game.global_flags[608] = 1
	return


def set_hickory_slides():
	game.moviequeue_add(418)
	game.moviequeue_add(419)
	game.moviequeue_play()
	return RUN_DEFAULT


def party_death_by_poison():
	if (game.global_vars[958] == 6):
		game.global_vars[958] = 7
		for pc in game.party:
			pc.critter_kill_by_effect()
			game.particles( "sp-Poison", pc )
	return 1


def hextor_wins():
	if ((game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6) and game.quests[97].state != qs_completed) or (game.global_vars[510] == 1):
	# you leave hommlet without killing all the hextorites after invasion has started or you leave Hommlet after turning down Ariakas
		game.quests[97].state = qs_botched 
		game.party[0].reputation_add( 53 )
		game.global_vars[510] = 2
		game.global_flags[504] = 1
	return


def verbo_tax( attachee, triggerer ):
	if (game.global_flags[549] == 1 and game.global_flags[826] == 0 and game.global_flags[260] == 0):
		if (game.party[0].money_get() >= 20000):
			game.party[0].money_adj(-20000)
			game.sound( 4178, 1 )
			game.party[0].float_mesfile_line( 'mes\\float.mes', 4 )
		else:
			game.global_vars[567] = game.global_vars[567] + 1
	game.global_flags[260] = 0
	return


##################################################################################
# EVERYTHING BELOW HAS TO DO WITH SITRA'S RANNOS AND GREMAG MOD AS FAR AS I KNOW #
##################################################################################


def council_heartbeat( attachee ):
	c_time = council_time()
	#1 - between 22:00 and 22:30 on council day
	#2 - between 22:30 and 23:00
	#3 - between 19:00 and 22:00
	#4 - after council events ( >23:00 and beyond that day), but without ordinary council
	#5 - ordinary council time
	#0 - otherwise
	if traders_awol() == 1:
		set_v(435,6)
	if c_time == 5:
		set_v(440,1)
	elif c_time == 1:
		if get_v(435) == 1:
			set_v(435,3)
		elif get_v(435) >= 5 or get_v(435) == 0 and get_v(440) == 0:
			set_v(440,1)
		elif get_v(435) == 4:
			#council_events()
			dummy = 1
	elif c_time == 2:
		if get_v(435) == 3 or get_v(435) == 1:
			set_v(435,4)
			set_v(436,1)
			game.global_flags[432] = 1
			#council_events()
	elif c_time == 3:
		if get_v(435) == 2:
			set_v(435,5)
			set_v(436,5)
			game.global_vars[750] = 3
			game.global_vars[751] = 3
			if (game.party[0].reputation_has(23) == 0 and (game.global_flags[814] == 0 or game.global_flags[815] == 0)):
				game.party[0].reputation_add(23)
	elif (c_time == 0 or c_time == 4):
		if c_time == 0 and game.global_flags[336] == 1 and (get_v(435) == 1 or get_v(435) == 2):
			set_v(435,0)
		elif get_v(435) == 3 or get_v(435) == 4 or (get_v(435) == 1 and c_time == 4):
		##chiefly used for the case where the whole thing played out without you
			if get_v(436) == 0:
				set_v(436,1)
			set_v(435,5)
			if (game.party[0].reputation_has(23) == 0 and (game.global_flags[814] == 0 or game.global_flags[815] == 0)):
				game.party[0].reputation_add(23)
		if npc_get(attachee, 10) == 0 and attachee.map == 5001 and c_time == 4:
			hommletans_go_home()
			npc_set(attachee,10)
		if get_v(435) == 2 and c_time == 4:
			set_v(435,5)
			set_v(436,5)
			game.global_vars[750] = 3
			game.global_vars[751] = 3
			if (game.party[0].reputation_has(23) == 0 and (game.global_flags[814] == 0 or game.global_flags[815] == 0)):
				game.party[0].reputation_add(23)
		set_v(440,0)
	return 1


def council_events():
### this script is fired from first_heartbeat in the Hommlet Exterior map 5001
	if ( game.leader.map == 5001 and game.global_vars[435] == 4 and game.global_flags[435] == 0 and game.global_flags[432] == 1 ):
	#### spawns everyone that was inside Towne Hall
		game.global_flags[435] = 1
		burne = game.obj_create(14453,location_from_axis(578,406))
		burne.scripts[10] = 0
		burne.scripts[19] = 0
		burne.obj_set_int(obj_f_critter_description_unknown,20000)
		burne.move(location_from_axis(578,406),0.0,0.0)
		burne.rotation = 1
		destroy_weapons(burne,4058,0,0)
		jaroo = game.obj_create(14005,location_from_axis(583,412))
		jaroo.scripts[10] = 0
		jaroo.scripts[19] = 0
		jaroo.obj_set_int(obj_f_critter_description_unknown,20001)
		jaroo.move(location_from_axis(583,412),0.0,0.0)
		jaroo.rotation = 0
		destroy_weapons(jaroo, 4047, 4111, 0)
		rufus = game.obj_create(14006,location_from_axis(571,407))
		rufus.scripts[10] = 0
		rufus.scripts[19] = 0
		rufus.obj_set_int(obj_f_critter_description_unknown,8071)
		rufus.move(location_from_axis(571,407),0.0,0.0)
		rufus.rotation = 3
		terjon = game.obj_create(14007,location_from_axis(570,412))
		terjon.scripts[10] = 0
		terjon.scripts[19] = 0
		terjon.obj_set_int(obj_f_critter_description_unknown,20003)
		terjon.move(location_from_axis(570,412),0.0,0.0)
		terjon.rotation = 4.5
		destroy_weapons(terjon,4124,6054,0)
		renton = game.obj_create(14012,location_from_axis(583,409))
		renton.scripts[10] = 0
		renton.scripts[19] = 0
		renton.obj_set_int(obj_f_critter_description_unknown,20007)
		renton.move(location_from_axis(583,409),0.0,0.0)
		renton.rotation = 1
		destroy_weapons(renton,4096,4036,6074)
		nevets = game.obj_create(14102,location_from_axis(576,407))
		nevets.scripts[10] = 0
		nevets.scripts[19] = 0
		nevets.obj_set_int(obj_f_critter_description_unknown,20058)
		nevets.move(location_from_axis(576,407),0.0,0.0)
		nevets.rotation = 3
		miller = game.obj_create(14031,location_from_axis(571,412))
		miller.scripts[10] = 0
		miller.scripts[19] = 0
		miller.obj_set_int(obj_f_critter_description_unknown,20026)
		miller.move(location_from_axis(571,412),3.0,0.0)
		miller.rotation = 3
		miller.condition_add_with_args( "Prone", 0, 0 )
		gundi = game.obj_create(14016,location_from_axis(582,411))
		gundi.scripts[10] = 0
		gundi.scripts[19] = 0
		gundi.obj_set_int(obj_f_critter_description_unknown,20011)
		gundi.move(location_from_axis(582,411),-10.0,-10.0)
		gundi.rotation = 3
		gundi.condition_add_with_args( "Prone", 0, 0 )
		crybaby = game.obj_create(14002,location_from_axis(575,417))
		crybaby.move(location_from_axis(575,417),0.0,0.0)
		crybaby.scripts[10] = 0
		crybaby.scripts[19] = 0
		crybaby.rotation = 5.5
		crybaby.scripts[19] = 0
		crybaby.object_flag_unset(OF_OFF)
		if (game.global_vars[436] == 4):
			game.timevent_add(proactivity,(crybaby,3000),2000)
		else:
			game.timevent_add(proactivity,(crybaby,3000),7000)

		randy1 = game.random_range(1,2)
		randy1 = 2
		#remove randomness for testing purposes
		if (randy1 == 1):
			gremag = game.obj_create(14014,location_from_axis(365,653))
			gremag.condition_add_with_args( "Invisible", 0, 0 )
			gremag.object_flag_set(OF_DONTDRAW)
			rannos = game.obj_create(14018,location_from_axis(366,655))
			rannos.condition_add_with_args( "Invisible", 0, 0 )
			rannos.object_flag_set(OF_DONTDRAW)
			dlg_popup = game.obj_create(14806,location_from_axis(364,653))
		else:
			gremag = game.obj_create(14014,location_from_axis(318,495))
			gremag.condition_add_with_args( "Invisible", 0, 0 )
			gremag.object_flag_set(OF_DONTDRAW)
			rannos = game.obj_create(14018,location_from_axis(320,496))
			rannos.condition_add_with_args( "Invisible", 0, 0 )
			rannos.object_flag_set(OF_DONTDRAW)
			dlg_popup = game.obj_create(14806,location_from_axis(317,494))
			#dlg_popup.object_flag_unset(OF_DONTDRAW)
			#dlg_popup.object_flag_unset(OF_CLICK_THROUGH)
		#### damage section, tough ones:
		if (game.global_vars[436] != 4):
			lightly_damage(renton)
			lightly_damage(terjon)
			lightly_damage(rufus)
			lightly_damage(jaroo)
			lightly_damage(burne)
			game.timevent_add(heal_script,(terjon,rufus),8500)
			game.timevent_add(heal_script,(jaroo,renton),9500)
		#### damage section, frail but important ones:
		if (game.global_vars[436] != 4):
			if (game.global_vars[436] != 6 and game.global_vars[436] != 7):
				lightly_damage(nevets)
				lightly_damage(gundi)
				game.timevent_add(heal_script,(jaroo,gundi),700)
				game.timevent_add(heal_script,(jaroo,nevets),5800)
			else:
				heavily_damage(nevets)
				heavily_damage(gundi)
		#### damage section, frail and unimportant ones:
		if (get_v(436) != 4):
			if (get_v(436) == 3):
				lightly_damage(miller)
				game.timevent_add(float_comment,(terjon,3000),3500)
				game.timevent_add(heal_script,(terjon,miller),3510)
			else:
				heavily_damage(miller)
	return


def is_follower(name):
	for obj in game.party:
		if (obj.name == name):
			return 1
	return 0


def hommletans_go_home( ):
	moshe = game.obj_create(14460,location_from_axis(577,412))
	for npc in game.obj_list_vicinity(moshe.location, OLC_NPC):
		if (to_be_deleted_outdoors(npc) == 1):
			npc.destroy()
	moshe.destroy()
	moshe = game.obj_create(14460,location_from_axis(365,653))
	for npc in game.obj_list_vicinity(moshe.location, OLC_NPC):
		if (to_be_deleted_outdoors(npc) == 1):
			npc.destroy()
	moshe.destroy()
	moshe = game.obj_create(14460,location_from_axis(318,495))
	for npc in game.obj_list_vicinity(moshe.location, OLC_NPC):
		if (to_be_deleted_outdoors(npc) == 1):
			npc.destroy()
	moshe.destroy()
	return


def to_be_deleted_outdoors(npc):
	#8008 - Gundigoot
	#8048 - Rannos
	#8049 - Gremag
	#8054 - burne (this is the correct name)
	#8071 - Rufus
	#14031 - Miller
	#14102 - Nevets
	#14371 - badger (DO NOT DELETE THEM OUTDOORS!)
	#14806 - Script device
	#20001 - Jaroo
	#20003 - Terjon
	#20007 - Renton
	if (npc.name == 8008 or npc.name == 8048 or npc.name == 8049 or npc.name == 8054 or npc.name == 8071 or npc.name == 14031 or npc.name == 14102 or npc.name == 20001 or npc.name == 20003 or npc.name == 20007):
		if (npc.leader_get() == OBJ_HANDLE_NULL):
			return 1
	return 0


def lightly_damage(npc):
	npc.damage(OBJ_HANDLE_NULL,D20DT_POISON,dice_new("1d6"))
	npc.damage(OBJ_HANDLE_NULL,D20DT_SUBDUAL,dice_new("3d3"),D20DAP_NORMAL)
	return


def heavily_damage(npc):
	#note: this script kills an NPC
	#since the san_dying is triggered, it makes the game think you killed him
	#so to avoid problems, reduce global_vars[23] (which counts the # of Hommeletans killed) beforehand
	if (game.global_vars[23] == 0):
		flag = 0
	else:
		flag = 1
		game.global_vars[23] = game.global_vars[23] - 1
	npc.damage(OBJ_HANDLE_NULL,D20DT_POISON,dice_new("30d1"))
	npc.damage(OBJ_HANDLE_NULL,D20DT_SUBDUAL,dice_new("15d1"))
	if (flag == 0 and game.global_vars[23] > 0):
		game.global_vars[23] = game.global_vars[23] - 1
	if (game.global_vars[23] < 2 and game.party[0].reputation_has(92) == 1):
		for pc in game.party:
			pc.reputation_remove( 92 )
	return


def destroy_weapons(npc, item1, item2, item3):
	if (item1 != 0):
		moshe = npc.item_find(item1)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item2 != 0):
		moshe = npc.item_find(item2)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item3 != 0):
		moshe = npc.item_find(item3)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	return


def heal_script(healer, target):
	if (obj_percent_hp(target) < 100):
		if (healer.name == 20003 and target.name == 14031):
			healer.cast_spell( spell_cure_moderate_wounds, target )
		if (healer.name == 20003 and target.name == 8071):
			healer.cast_spell( spell_cure_light_wounds, target )
		if (healer.name == 20001 and target.name == 8008):
			healer.cast_spell( spell_cure_critical_wounds, target )
		if (healer.name == 20001 and target.name == 14102):
			healer.cast_spell( spell_cure_serious_wounds, target )
		if (healer.name == 20001 and target.name == 20007):
			healer.cast_spell( spell_cure_moderate_wounds, target )
		game.timevent_add(heal_script,(healer,target),700)
	return


def float_comment(attachee, line):
	attachee.float_line(line,game.leader)
	return


def proactivity(npc,line_no):
	npc.turn_towards(game.party[0])
	if (critter_is_unconscious(game.party[0]) != 1 and game.party[0].type == obj_t_pc and game.party[0].d20_query(Q_Prone) == 0 and npc.can_see(game.party[0])):
		game.party[0].begin_dialog(npc,line_no)
	else:
		for pc in game.party:
			npc.turn_towards(pc)
			if (critter_is_unconscious(pc) != 1 and pc.type == obj_t_pc and pc.d20_query(Q_Prone) == 0 and npc.can_see(pc)):
				pc.begin_dialog(npc,line_no)
	return
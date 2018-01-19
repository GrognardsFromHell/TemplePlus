from toee import *
from utilities import *
from Livonya import get_melee_reach_strategy
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	attachee.turn_towards(triggerer)
	if (game.leader.reputation_has(32) == 1 or game.leader.reputation_has(30) == 1 or game.leader.reputation_has(29) == 1):
		attachee.float_line(11004,triggerer)
	elif ((game.quests[3].state == qs_completed) or (game.quests[4].state == qs_completed)):
		triggerer.begin_dialog( attachee, 20 )
	elif ((attachee.has_met( triggerer )) or (game.quests[3].state == qs_accepted) or (game.quests[4].state == qs_accepted)):
		triggerer.begin_dialog( attachee, 10 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_vars[23] = game.global_vars[23] + 1
	if (game.global_vars[23] >= 2):
		game.party[0].reputation_add( 92 )
	return RUN_DEFAULT


###########################################################################################
## THIS IS DEFAULT SCRIPT TO GET TWO WEAPON FIGHTERS TO USE TWO WEAPONS (this is needed) ##
###########################################################################################

def san_enter_combat( attachee, triggerer ):
	if ((attachee.name == 14238 or attachee.name == 14697 or attachee.name == 14573 or attachee.name == 14824) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
		# Ettin
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14336 and (not attachee.has_wielded(4081) or not attachee.has_wielded(4126))):
		# Elven Ranger
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14357 and (not attachee.has_wielded(4156) or not attachee.has_wielded(4159))):
		# Grank's Bandit
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14747 and (not attachee.has_wielded(4040) or not attachee.has_wielded(4159))):
		# Half Orc Assassin
		attachee.item_wield_best_all()
		game.new_sid = 0
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if game.global_flags[403] == 1 and game.global_flags[405] == 1:
		attachee.float_mesfile_line( 'mes\\script_activated.mes', 15, 1 )

	if (attachee.name in [14262, 14191, 14691]): # trolls
		if (attachee.condition_add("Rend")):
			print "Added Rend"
	if (attachee.name in [14050, 14391, 14352, 14406, 14408, 14610, 14994]): # wolves
		if (attachee.condition_add("Tripping Bite")):
			print "Added Tripping Bite"
	
	if ((attachee.name == 14238 or attachee.name == 14697 or attachee.name == 14573 or attachee.name == 14824) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
	# Ettin
		attachee.item_wield_best_all()
#		game.new_sid = 0
	elif (attachee.name == 14336 and (not attachee.has_wielded(4081) or not attachee.has_wielded(4126))):
	# Elven Ranger
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
#		game.new_sid = 0
	elif (attachee.name == 14357 and (not attachee.has_wielded(4156) or not attachee.has_wielded(4159))):
	# Grank's Bandit
		attachee.item_wield_best_all()
#		game.new_sid = 0
	elif (attachee.name == 14747 and (not attachee.has_wielded(4040) or not attachee.has_wielded(4159))):
	# Half Orc Assassin
		attachee.item_wield_best_all()
	elif attachee.name == 14057 and attachee.map == 5002:
	# Giant Frogs of the Moathouse
		for obj in game.obj_list_vicinity(location_from_axis(492, 522), OLC_NPC):
			if obj.name == 14057 and obj.leader_get() == OBJ_HANDLE_NULL:
				obj.attack(game.leader)
		for obj in game.obj_list_vicinity(location_from_axis(470, 522), OLC_NPC):
			if obj.name == 14057 and obj.leader_get() == OBJ_HANDLE_NULL:
				obj.attack(game.leader)
		for obj in game.obj_list_vicinity(location_from_axis(473, 506), OLC_NPC):
			if obj.name == 14057 and obj.leader_get() == OBJ_HANDLE_NULL:
				obj.attack(game.leader)
	elif attachee.name == 14337 and attachee.map == 5066:
	# Earth Temple Guards
		if attachee.map == 5066: # temple level 1 - guards near the corridor of bones - call in Earth Temple Fighters
			xx,yy = location_to_axis( attachee.location )
			if ( (xx-441)**2 + (yy - 504)**2 ) < 200:
				for npc in game.obj_list_vicinity(location_from_axis(441, 504), OLC_NPC):
					if npc.name in [14337, 14338] and npc.leader_get() == OBJ_HANDLE_NULL and npc.is_unconscious() == 0:
						xx2,yy2 = location_to_axis( npc.location )
						if ( (xx2-441)**2 + (yy2 - 504)**2 ) < 300:
							#print str(xx) + " " + str(yy) + "  " + str(xx2) + " " + str(yy2)
							if npc.scripts[15] == 0:
								npc.scripts[15] = 2
							npc.attack(game.leader)
	elif attachee.name == 14338: # Earth Temple Fighters
		if attachee.item_worn_at(3) == OBJ_HANDLE_NULL:
			attachee.item_wield_best_all()
	elif attachee.name == 14888: # siren cultist screaching
		game.sound( 4188, 1 )


	## ALSO, THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)

	if attachee.leader_get() != OBJ_HANDLE_NULL: # Don't wanna fuck up charmed enemies
		return RUN_DEFAULT

	#################################
	# Special scripting		#
	#################################

	if attachee.name == 14262 or attachee.name == 14195 or attachee.name == 14343 or attachee.name == 14224 or attachee.name == 14375:
		# 14262 - Troll, 14195 - Oohlgrist, 14343 - Hydra, 14224 - Hydra Keeper
		# 14375 - Water Snake
		get_melee_reach_strategy(attachee)
		print "strategy chosen was " + str(attachee.obj_get_int(obj_f_critter_strategy))
	elif attachee.name == 8091: # Belsornig
		goodguys = 0
		nonevilguys = 0
		for dude in game.party:
			if dude.stat_level_get(stat_alignment) & ALIGNMENT_GOOD != 0:
				goodguys = goodguys + 1
			if dude.stat_level_get(stat_alignment) & (ALIGNMENT_EVIL) == 0:
				nonevilguys = nonevilguys + 1
		if goodguys <= 1 or (nonevilguys) <= 3:
			attachee.obj_set_int(obj_f_critter_strategy, 74) # Belsornig AI without Unholy Blight
	elif attachee.name == 14888: # Siren Cultist
		if party_closest(attachee, conscious_only= 1, mode_select= 0, exclude_warded= 0, exclude_charmed = 1) == OBJ_HANDLE_NULL:
			attachee.obj_set_int(obj_f_critter_strategy, 573) # No PCs left to charm; charming NPCs causes them to be permanently hostile, essentially killing them unless charmed back or dominated, so I'll switch to an alternative spell list here
		else:
			attachee.obj_set_int(obj_f_critter_strategy, 572) # the charm tactic
	elif attachee.name in [14095, 14128, 14129] and attachee.map == 5066:
		xx, yy = location_to_axis(attachee.location)
		if (yy <= 548 and yy >= 540 and xx >= 416 and xx <= 425) or (yy <= 540 and yy >= 530 and xx >= 421 and xx <= 425) or (xx <= 419 and yy <= 533 and yy >= 518): # Room to the west of the harpy corridor
			if attachee.d20_query(Q_Turned):
				for obj in game.obj_list_vicinity(attachee.location, OLC_PORTAL):
					obj.object_flag_set(OF_OFF)
				
			
	#############################################################
	# Summoned Creaute/ Spiritual Weapon / Warded PC scripting	#
	#############################################################
	Spiritual_Weapon_Begone( attachee )
	
		
	return RUN_DEFAULT


###################################################################################################
## THIS IS DEFAULT SCRIPT TO GET TWO WEAPON FIGHTERS TO USE TWO WEAPONS (this may not be needed) ##
###################################################################################################

def san_heartbeat( attachee, triggerer ):
	if (game.combat_is_active()):
		return RUN_DEFAULT
	if ((attachee.name == 14238 or attachee.name == 14697 or attachee.name == 14573 or attachee.name == 14824) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
		# Ettin
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14336 and attachee.item_find(4058) != OBJ_HANDLE_NULL):
		# Elven Ranger
		itemA = attachee.item_find(4058)
		itemA.destroy()
		create_item_in_inventory( 4126, attachee )
	if (attachee.name == 14336 and (not attachee.has_wielded(4081) or not attachee.has_wielded(4126))):
		# Elven Ranger
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14357 and attachee.item_find(4087) != OBJ_HANDLE_NULL):
		# Grank's Bandit
		itemA = attachee.item_find(4087)
		itemA.destroy()
		itemA = attachee.item_find(5004)
		itemA.destroy()
	if (attachee.name == 14357 and (not attachee.has_wielded(4156) or not attachee.has_wielded(4159))):
		# Grank's Bandit
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
		game.new_sid = 0
	if (attachee.name == 14747 and (not attachee.has_wielded(4040) or not attachee.has_wielded(4159))):
		# Half Orc Assassin
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
		game.new_sid = 0
	return RUN_DEFAULT


def san_disband( attachee, triggerer ):
	for pc in game.party:
		attachee.ai_shitlist_remove( pc )
		attachee.reaction_set( pc, 50 )
	return RUN_DEFAULT
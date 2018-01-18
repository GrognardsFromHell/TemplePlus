from utilities import *
from toee import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):

	if (attachee.map == 5085 and game.party[0].reputation_has( 23 ) == 1 and game.global_flags[94] == 1 and (game.global_flags[815] == 0 or game.global_flags[814] == 0)):	
		attachee.object_flag_set(OF_OFF)	
		party_level = group_average_level(game.leader)
		game.global_vars[759] = game.global_vars[759] + 1
		if ((game.global_vars[759] <= 2 and game.random_range(1,4) <= 3) or party_level <= 5):
			return RUN_DEFAULT
		else:
			game.global_flags[851] = 1
			if (game.global_flags[836] == 0):
				assassin = game.obj_create( 14303, location_from_axis (468L, 496L))	## location 1
				assassin.turn_towards(triggerer)
				assassin.item_wield_best_all()
				assassin.concealed_set(13)
			if (game.global_flags[836] == 1):
				assassin = game.obj_create( 14613, location_from_axis (477L, 487L))		## location 2
				assassin.rotation = 2.5
				assassin.item_wield_best_all()
			if (game.global_flags[815] == 0 and game.global_flags[814] == 0):
				rannos = game.obj_create( 14611, location_from_axis (475L, 486L))		## location 7
				gremag = game.obj_create( 14612, location_from_axis (479L, 485L))		## location 6
				rannos.rotation = 2.5
				gremag.rotation = 2.5
				rannos.item_wield_best_all()
				gremag.item_wield_best_all()
			if (game.global_flags[815] == 1 and game.global_flags[814] == 0):
				rannos = game.obj_create( 14611, location_from_axis (475L, 486L))		## location 7
				hired = game.obj_create( 14613, location_from_axis (479L, 485L))		## location 6
				rannos.rotation = 2.5
				rannos.item_wield_best_all()
				hired.rotation = 2.5
				hired.item_wield_best_all()
			if (game.global_flags[815] == 0 and game.global_flags[814] == 1):
				gremag = game.obj_create( 14612, location_from_axis (475L, 486L))		## location 7
				hired = game.obj_create( 14613, location_from_axis (479L, 485L))		## location 6
				gremag.rotation = 2.5
				gremag.item_wield_best_all()
				hired.rotation = 2.5
				hired.item_wield_best_all()
			thug = game.obj_create( 14606, location_from_axis (477L, 488L))			## location 2
			thug.rotation = 2.5
			thug.item_wield_best_all()
			barb = game.obj_create( 14608, location_from_axis (474L, 489L))			## location 5
			barb.rotation = 2.5
			barb.item_wield_best_all()
			rr = game.random_range(1,2)
			if (rr == 1):
				cleric = game.obj_create( 14609, location_from_axis (470L, 484L))		## location 4
				mage = game.obj_create( 14607, location_from_axis (478L, 481L))		## location 3
				cleric.rotation = 2.5
				mage.rotation = 2.5
			if (rr == 2):
				cleric = game.obj_create( 14609, location_from_axis (478L, 481L))		## location 3
				mage = game.obj_create( 14607, location_from_axis (470L, 484L))		## location 4
				cleric.rotation = 2.5
				mage.rotation = 2.5
			mage.item_wield_best_all()
			cleric.item_wield_best_all()
			if (game.global_flags[815] == 0 and game.global_flags[814] == 0):
				leader = game.party[0]
				leader.begin_dialog( rannos, 2000 )
				return SKIP_DEFAULT
			if (game.global_flags[815] == 1 and game.global_flags[814] == 0):
				leader = game.party[0]
				leader.begin_dialog( rannos, 2100 )
				return SKIP_DEFAULT
			if (game.global_flags[815] == 0 and game.global_flags[814] == 1):
				leader = game.party[0]
				leader.begin_dialog( gremag, 2100 )
				return SKIP_DEFAULT
			

	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	attachee.remove_from_initiative()
	attachee.object_flag_set(OF_OFF)
	return SKIP_DEFAULT


def san_start_combat( attachee, triggerer ):
	attachee.remove_from_initiative()
	attachee.object_flag_set(OF_OFF)
	return SKIP_DEFAULT


def san_heartbeat( attachee, triggerer ):

	if (attachee.map == 5078):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (obj.distance_to(attachee) <= 15):
				obj.begin_dialog( attachee, 2100 )
		return SKIP_DEFAULT

	if (attachee.map == 5015 and game.global_flags[37] == 0 and game.global_flags[835] == 1):
		attachee.destroy()
		npc = game.obj_create( 14371, location_from_axis (477L, 488L))
		npc2 = game.obj_create( 14371, location_from_axis (481L, 475L))
		npc3 = game.obj_create( 14371, location_from_axis (486L, 479L))
		npc4 = game.obj_create( 14371, location_from_axis (482L, 486L))



	if (attachee.map == 5014 and game.global_flags[37] == 0 and game.global_flags[835] == 1):
		attachee.destroy()
#		chains = game.obj_create( 2135, location_from_axis (491L, 484L))
#		chains.rotation = 2.5
#		chains = game.obj_create( 2135, location_from_axis (491L, 484L))
#		chains.rotation = 4.5
#		chains = game.obj_create( 2135, location_from_axis (491L, 484L))
#		chains.rotation = 3.5
		npc = game.obj_create( 14614, location_from_axis (490L, 483L))
#		npc2 = game.obj_create( 14607, location_from_axis (490L, 483L))
#		loc = npc.location
#		npc.stat_level_get(stat_subdual_damage)
#		npc.damage(OBJ_HANDLE_NULL,D20DT_SUBDUAL,dice_new("7d1"),D20DAP_NORMAL)
		npc.damage( OBJ_HANDLE_NULL, 0, dice_new("52d1"))
		game.global_vars[758] = 0
#		game.particles( "sp-Hold Person", npc )
#		game.particles( "sp-Bestow Curse", npc )
#		npc2.cast_spell(spell_hold_person, npc)
#		npc.condition_add_with_args( 'sp-Tashas Hideous Laughter', OBJ_HANDLE_NULL, 50000, 0 )
#		npc.condition_add_with_args("Prone",0,5000)




	return RUN_DEFAULT


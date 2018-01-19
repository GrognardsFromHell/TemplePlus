from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog(attachee,1)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_vars[30] = game.global_vars[30] + 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )
	# if (attachee.name == 14361 or attachee.name == 14258):
		# if (game.global_vars[762] == 0):
			# damage_dice = dice_new( '1d8' )
			# game.particles( 'Mon-Vrock-Spores', attachee)
			# for obj in game.obj_list_vicinity(attachee.location,OLC_CRITTERS):
				# if (obj.distance_to(attachee) <= 10 and obj.name != 14258 and obj.name != 14259 and obj.name != 14263 and obj.name != 14286 and obj.name != 14358 and obj.name != 14359 and obj.name != 14360 and obj.name != 14361 and obj.name != 14110):
					# obj.spell_damage( attachee, D20DT_POISON, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, 261 )
					# obj.condition_add_with_args( 'sp-Vrock Spores', 273, 10, 0)
					# game.particles( 'Mon-Vrock-Spores-Hit', obj )
		# game.global_vars[762] = game.global_vars[762] + 1
		# if (game.global_vars[762] == 3):
			# game.global_vars[762] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				attachee.turn_towards(obj)
				obj.begin_dialog(attachee,1)
				game.new_sid = 0
				return RUN_DEFAULT
		#if (game.global_vars[712] == 0 and attachee.leader_get() == OBJ_HANDLE_NULL and attachee.distance_to(party_closest(attachee) ) <= 30  ):
		#	attachee.spells_pending_to_memorized()
		#	attachee.cast_spell(spell_heroism, attachee)
		#	game.global_vars[712] = 1
		# The motherfucker doesn't want to cast spells out of combat. Hmpf!
	return RUN_DEFAULT
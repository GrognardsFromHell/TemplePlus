from __main__ import game;
from toee import *
from combat_standard_routines import *


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.name == 14342 and attachee.item_find(4083) == OBJ_HANDLE_NULL):		## Lamia
		create_item_in_inventory( 4083, attachee )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary!
	#	create_item_in_inventory( 8903, attachee )

	if (attachee.name == 14328 and game.random_range(1,100) <= 40):		## Bodak Death Gaze
		attachee.spells_pending_to_memorized()
	
	if (attachee.name == 14309 and game.random_range(1,100) <= 25):		## Gorgon Breath Attack
		attachee.spells_pending_to_memorized()

	if (attachee.name == 14109 and game.random_range(1,100) <= 25):		## Ice Lizard Breath Attack
		attachee.spells_pending_to_memorized()

	if (attachee.name == 14342 and attachee.has_wielded(4083)):		## Lamia
		attachee.item_find(4083).destroy()
		return RUN_DEFAULT
	if (attachee.name == 14342 and attachee.item_find(4083) == OBJ_HANDLE_NULL and game.random_range(1,100) <= 50):
		create_item_in_inventory( 4083, attachee )
		attachee.item_wield_best_all()

	if (attachee.name == 14295 and not attachee.d20_query(Q_Critter_Is_Blinded)):	## Basilisk
		attachee.spells_pending_to_memorized()

	if (attachee.name == 14258):		## Guardian Vrock
		game.global_vars[762] = game.global_vars[762] + 1
		if (game.global_vars[762] >= 3):
			damage_dice = dice_new( '1d8' )
			game.particles( 'Mon-Vrock-Spores', attachee)
			for obj in game.obj_list_vicinity(attachee.location,OLC_CRITTERS):
				if (obj.distance_to(attachee) <= 10 and obj.name != 14258 and obj.name != 14361):
					obj.spell_damage( attachee, D20DT_POISON, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, 261 )
					game.particles( 'Mon-Vrock-Spores-Hit', obj )
	
#		damage_dice = dice_new( '1d8' )
#		game.particles( 'Mon-Vrock-Spores', attachee)
#		for obj in game.obj_list_vicinity(attachee.location,OLC_CRITTERS):
#			if (obj.distance_to(attachee) <= 10 and obj.name != 14258):
#				obj.damage( OBJ_HANDLE_NULL, D20DT_POISON, damage_dice, D20DAP_NORMAL)
#				obj.spell_damage( attachee, D20DT_POISON, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, 261 )
#				obj.condition_add_with_args( "Poisoned", 273 , 10)
#				obj.condition_add_with_args( 'sp-Vrock Spores', 273, 10, 0)
					 
#				game.particles( 'Mon-Vrock-Spores-Hit', obj )

#	if ((attachee.name == 14259 or attachee.name == 14360) and game.random_range(1,3) <= 3):
#		create_item_in_inventory( 8909, attachee )

	#################################
	# Spiritual Weapon Shenanigens	#
	#################################
	Spiritual_Weapon_Begone( attachee )
	
	return RUN_DEFAULT
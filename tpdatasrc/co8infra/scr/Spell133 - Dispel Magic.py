from toee import *
from utilities import  * 

def OnBeginSpellCast( spell ):
	print "Dispel Magic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def OnSpellEffect ( spell ):
		
	# Lareth Special scripting in the Moathouse
	if spell.caster.name == 8002 and spell.caster.map == 5005: 
		player_cast_web_obj = OBJ_HANDLE_NULL
		player_cast_entangle_obj = OBJ_HANDLE_NULL
		for spell_obj in game.obj_list_vicinity(spell.caster.location, OLC_GENERIC):
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 531 + (1<<15):
				player_cast_web_obj = spell_obj
			if spell_obj.obj_get_int(obj_f_secretdoor_dc) == 153 + (1<<15):
				player_cast_entangle_obj = spell_obj
		
		if player_cast_entangle_obj != OBJ_HANDLE_NULL and player_cast_web_obj != OBJ_HANDLE_NULL and player_cast_entangle_obj.distance_to(player_cast_web_obj) <= 18:
			locc_ = ( player_cast_entangle_obj.location + player_cast_web_obj.location ) / 2
		elif player_cast_web_obj != OBJ_HANDLE_NULL:
			locc_ = player_cast_web_obj.location
		elif player_cast_entangle_obj != OBJ_HANDLE_NULL:
			locc_ = player_cast_entangle_obj.location
		else:
			locc_ = spell.caster.location
			
		
		game.particles( 'sp-Dispel Magic - Area', locc_ )
		dispel_obj = game.obj_create( OBJECT_SPELL_GENERIC, locc_)
		dispel_obj.move( locc_, 0, 0 )

		for target in game.obj_list_vicinity(locc_, OLC_GENERIC ):
			if target.distance_to(dispel_obj) <= 20:
				partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target )
				#aaa1 = game.party[0].damage( OBJ_HANDLE_NULL, 0, dice_new("1d3"))
				target.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 1 )
				#game.party[0].damage( OBJ_HANDLE_NULL, 0, dice_new("1d4"))
				
		for target in game.obj_list_vicinity(locc_, OLC_NPC + OLC_PC):
			if target.distance_to(dispel_obj) <= 18:
				if (target.type == obj_t_pc) or (target.type == obj_t_npc) or (target.type == obj_t_generic):
					partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target )
					target.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, partsys_id )
					
		dispel_obj.destroy()
	# check if we are targetting an object or an area
	elif spell.is_object_selected() == 1:
		target = spell.target_list[0]

		# support dispel on critters
		if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):
			target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
			target.obj.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 0 )

		# support dispel on portals and containers
		elif (target.obj.type == obj_t_portal) or (target.obj.type == obj_t_container):
			if target.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
				target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
				target.obj.portal_flag_unset( OPF_MAGICALLY_HELD )
				spell.target_list.remove_target( target.obj )

		# support dispel on these obj_types: weapon, ammo, armor, scroll
		# NO support for: money, food, key, written, generic, scenery, trap, bag
		#elif (target.obj.type == obj_t_weapon) or (target.obj.type == obj_t_ammo) or (target.obj.type == obj_t_armor) or (target.obj.type == obj_t_scroll):
			#print "[dispel magic] - items not supported yet!"
			#game.particles( 'Fizzle', target.obj )
			#spell.target_list.remove_target( target.obj )

	else:
		# draw area effect particles
		game.particles( 'sp-Dispel Magic - Area', spell.target_loc )

		for target in spell.target_list:

			if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):
				target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
				target.obj.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 1 )

			# support dispel on portals and containers
			elif (target.obj.type == obj_t_portal) or (target.obj.type == obj_t_container):
				if target.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
					target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
					target.obj.portal_flag_unset( OPF_MAGICALLY_HELD )
					spell.target_list.remove_target( target.obj )

			# support dispel on these obj_types: weapon, ammo, armor, scroll
			# NO support for: money, food, key, written, generic, scenery, trap, bag
			#elif (target.obj.type == obj_t_weapon) or (target.obj.type == obj_t_ammo) or (target.obj.type == obj_t_armor) or (target.obj.type == obj_t_scroll):
				#print "[dispel magic] - items not supported yet!"
				#game.particles( 'Fizzle', target.obj )
				#spell.target_list.remove_target( target.obj )

	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	spell.spell_end(spell.id, 1)
	print "Dispel Magic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dispel Magic OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Dispel Magic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def OnSpellEffect( spell ):

	spell.caster_level = min( 20, spell.caster_level )

	# check if we are targetting an object or an area
	if spell.is_object_selected() == 1:
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

	spell.spell_end( spell.id , 1)

def OnBeginRound( spell ):
	spell.spell_end(spell.id, 1)
	print "Dispel Magic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dispel Magic OnEndSpellCast"
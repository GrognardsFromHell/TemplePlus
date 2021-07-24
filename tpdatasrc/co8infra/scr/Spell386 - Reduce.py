from toee import *

def OnBeginSpellCast( spell ):
	print "Reduce OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Reduce OnSpellEffect"

	spell.duration = 10 * spell.caster_level
	target_item = spell.target_list[0]

	if target_item.obj.is_friendly( spell.caster ):
		if target_item.obj.is_category_type( mc_type_humanoid ):
			return_val = target_item.obj.condition_add_with_args( 'sp-Reduce', spell.id, spell.duration, 0 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Reduce Person', target_item.obj )
			else:
				# sp-Reduce not added, probably dispelled sp-Enlarge
				spell.target_list.remove_target(target_item.obj)

		else:
			# not a humanoid
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	else:
		if target_item.obj.is_category_type( mc_type_humanoid ):
			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):

				# saving throw unsuccesful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				return_val = target_item.obj.condition_add_with_args( 'sp-Reduce', spell.id, spell.duration, 0 )
				if return_val == 1:
					target_item.partsys_id = game.particles( 'sp-Reduce Person', target_item.obj )
				else:
					# sp-Reduce not added, probably dispelled sp-Enlarge
					spell.target_list.remove_target(target_item.obj)

			else:

				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )
		else:
			# not a humanoid
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Reduce OnBeginRound"

def OnEndSpellCast( spell ):
	print "Reduce OnEndSpellCast"
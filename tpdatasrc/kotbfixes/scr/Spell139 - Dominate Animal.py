from toee import *

def OnBeginSpellCast( spell ):
	print "Dominate Animal OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Dominate Animal OnSpellEffect"

	spell.duration = 1 * spell.caster_level
	target_item = spell.target_list[0]

	if not target_item.obj.is_friendly( spell.caster ):
		if target_item.obj.is_category_type( mc_type_animal ):

			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				target_item.obj.condition_add_with_args( 'sp-Dominate Animal', spell.id, spell.duration, target_item.obj.hit_dice_num )
				target_item.partsys_id = game.particles( 'sp-Dominate Animal', target_item.obj )

				# add target to initiative, just in case
				target_item.obj.add_to_initiative()
				game.update_combat_ui()

			else:

				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )

		else:
			# not an animal
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31002 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	else:

		# can't target friendlies
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Dominate Animal OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dominate Animal OnEndSpellCast"
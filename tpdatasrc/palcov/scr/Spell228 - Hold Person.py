from toee import *

def OnBeginSpellCast( spell ):
	print "Hold Person OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Hold Person OnSpellEffect"

	if spell.caster.name in [14927]:	# Avoral	
		spell.dc = 16			# 10 + 3 + 3 (charisma)

	spell.duration = 1 * spell.caster_level

	target_item = spell.target_list[0]

	if target_item.obj.is_category_type( mc_type_humanoid ):

		if target_item.obj.get_size < STAT_SIZE_LARGE:

			# allow Will saving throw to negate
			if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )
			else:
				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				# HTN - apply condition HOLD (paralyzed)
				target_item.obj.condition_add_with_args( 'sp-Hold Person', spell.id, spell.duration, 0 )
				target_item.partsys_id = game.particles( 'sp-Hold Person', target_item.obj )
		else:
			# not medium sized or smaller
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31005 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	else:
		# not a person
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Hold Person OnBeginRound"

def OnEndSpellCast( spell ):
	print "Hold Person OnEndSpellCast"
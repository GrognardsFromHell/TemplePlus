from toee import *

def OnBeginSpellCast( spell ):
	print "Bane OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Bane OnSpellEffect"

	remove_list = []

	spell.duration = 10 * spell.caster_level
	game.particles( 'sp-Bane', spell.caster )

	for target_item in spell.target_list:
		#print "target " + str(target_item.obj)

		if not target_item.obj.is_friendly( spell.caster ):

			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				return_val = target_item.obj.condition_add_with_args( 'sp-Bane', spell.id, spell.duration, 0 )
				if return_val == 1:
					target_item.partsys_id = game.particles( 'sp-Bane Hit', target_item.obj )

			else:

				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				remove_list.append( target_item.obj )

		else:

			remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Bane OnBeginRound"

def OnEndSpellCast( spell ):
	print "Bane OnEndSpellCast"
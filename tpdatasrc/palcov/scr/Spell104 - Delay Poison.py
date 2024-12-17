from toee import *

def OnBeginSpellCast( spell ):
	print "Delay Poison OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Delay Poison OnSpellEffect"

	spell.duration = 600 * spell.caster_level
	target_item = spell.target_list[0]

	if target_item.obj.is_friendly( spell.caster ):
		target_item.obj.condition_add_with_args( 'sp-Delay Poison', spell.id, spell.duration, 0 )
		target_item.partsys_id = game.particles( 'sp-Delay Poison', target_item.obj )
	else:
		if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):

			# saving throw unsuccesful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			target_item.obj.condition_add_with_args( 'sp-Delay Poison', spell.id, spell.duration, 0 )
			target_item.partsys_id = game.particles( 'sp-Delay Poison', target_item.obj )

		else:

			# saving throw successful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Delay Poison OnBeginRound"

def OnEndSpellCast( spell ):
	print "Delay Poison OnEndSpellCast"
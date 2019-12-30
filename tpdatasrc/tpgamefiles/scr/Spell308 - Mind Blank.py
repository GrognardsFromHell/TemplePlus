from toee import *

def OnBeginSpellCast( spell ):
	print "Mind Blank OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	spell.duration = 24 * 600 #24 hour effect
	target = spell.target_list[0]

	if target.obj.is_friendly( spell.caster ):

		target.obj.condition_add_with_args( 'sp-Mind Blank', spell.id, spell.duration)
		target.partsys_id = game.particles( 'sp-Mind Blank', target.obj )

	elif not target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

		# saving throw unsuccesful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		target.obj.condition_add_with_args( 'sp-Mind Blank', spell.id, spell.duration)
		target.partsys_id = game.particles( 'sp-Mind Blank', target.obj )

	else:

		# saving throw successful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

		game.particles( 'Fizzle', target.obj )
		spell.target_list.remove_target( target.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Mind Blank OnBeginRound"

def OnEndSpellCast( spell ):
	print "Mind Blank OnEndSpellCast"
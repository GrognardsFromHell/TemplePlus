from toee import *
from utilities import  *

def OnBeginSpellCast( spell ):
	print "Neutralize Poison OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Neutralize Poison OnSpellEffect"

	spell.duration = 100 * spell.caster_level
	
	target = spell.target_list[0]

	if target.obj.is_friendly( spell.caster ):
		#target.partsys_id = game.particles( 'sp-Neutralize Poison', target.obj )
		target.obj.condition_add_with_args( 'sp-Neutralize Poison', spell.id, spell.duration, 0 )
		game.particles( 'sp-Neutralize Poison', target.obj ) # not a persistent particle effect anyway		

	elif not target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

		# saving throw unsuccesful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		#target.partsys_id = game.particles( 'sp-Neutralize Poison', target.obj )
		target.obj.condition_add_with_args( 'sp-Neutralize Poison', spell.id, spell.duration, 0 )
		game.particles( 'sp-Neutralize Poison', target.obj ) # not a persistent particle effect anyway

	else:

		# saving throw successful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

		game.particles( 'Fizzle', obj )
		spell.target_list.remove_target( target.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Neutralize Poison OnBeginRound"

def OnEndSpellCast( spell ):
	print "Neutralize Poison OnEndSpellCast"
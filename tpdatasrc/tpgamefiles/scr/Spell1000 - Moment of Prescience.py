from toee import *

def OnBeginSpellCast( spell ):
	print "Moment of Prescience OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-divination-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Moment of Prescience OnSpellEffect"

	spell.duration = 600 * spell.caster_level

	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Moment of Prescience', spell.id, spell.duration, 0 )
	target_item.partsys_id = game.particles( 'sp-Moment of Prescience', target_item.obj )
	
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Moment of Prescience OnBeginRound"

def OnEndSpellCast( spell ):
	print "Moment of Prescience OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Foresight OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-divination-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Foresight OnSpellEffect"

	spell.duration = 100 * spell.caster_level
	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Foresight', spell.id, spell.duration, 0 )
	target_item.partsys_id = game.particles( 'sp-Foresight', target_item.obj )

def OnBeginRound( spell ):
	print "Foresight OnBeginRound"

def OnEndSpellCast( spell ):
	print "Foresight OnEndSpellCast"
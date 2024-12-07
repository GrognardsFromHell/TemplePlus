from toee import *

from Co8 import *

def OnBeginSpellCast( spell ):
	print "Raise Dead OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Raise Dead OnSpellEffect"

	spell.duration = 0
	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Raise Dead', spell.id, spell.duration, 0 )
	#target_item.partsys_id = game.particles( 'sp-Raise Dead', target_item.obj )
	
	spell.spell_end( spell.id, 1 )

def OnBeginRound( spell ):
	spell.spell_end( spell.id, 1 )
	print "Raise Dead OnBeginRound"

def OnEndSpellCast( spell ):
	print "Raise Dead OnEndSpellCast"

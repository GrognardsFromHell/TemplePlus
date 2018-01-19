from toee import *

def OnBeginSpellCast( spell ):
	print "Remove Curse OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Remove Curse OnSpellEffect"

	spell.duration = 0

	target = spell.target_list[0]

	game.particles( 'sp-Remove Curse', target.obj )

	target.partsys_id = game.particles( 'sp-Remove Curse', target.obj )
	target.obj.condition_add_with_args( 'sp-Remove Curse', spell.id, spell.duration, 0 )

	spell.spell_end(spell.id, 1)

def OnBeginRound( spell ):
	print "Remove Curse OnBeginRound"

def OnEndSpellCast( spell ):
	print "Remove Curse OnEndSpellCast"
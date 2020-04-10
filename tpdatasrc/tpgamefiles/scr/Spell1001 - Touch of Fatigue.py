from toee import *

def OnBeginSpellCast( spell ):
	print "Touch of Fatigue OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Touch of Fatigue OnSpellEffect"

	spell.duration = spell.caster_level
	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Touch of Fatigue', spell.id, spell.duration, 1 )
	target_item.partsys_id = game.particles( 'sp-Touch of Fatigue', target_item.obj )

def OnBeginRound( spell ):
	print "Touch of Fatigue OnBeginRound"

def OnEndSpellCast( spell ):
	print "Touch of Fatigue OnEndSpellCast"
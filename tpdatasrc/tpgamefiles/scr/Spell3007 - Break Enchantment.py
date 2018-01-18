from toee import *

def OnBeginSpellCast( spell ):
	print "Song of Freedom OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Song of Freedom OnSpellEffect"

	remove_list = []

	spell.duration = 0

	game.particles( 'sp-Break Enchantment-Area', spell.caster )
	for target_item in spell.target_list:

		target_item.obj.condition_add_with_args( 'sp-Break Enchantment', spell.id, spell.duration, 0 )
		target_item.partsys_id = game.particles( 'sp-Break Enchantment-Hit', target_item.obj )

		remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	spell.spell_end(spell.id, 1)
	print "Song of Freedom OnBeginRound"

def OnEndSpellCast( spell ):
	print "Song of Freedom OnEndSpellCast"
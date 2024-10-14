from toee import *
from casters import staff_has, staff_stats

def OnBeginSpellCast( spell ):
	print "Mislead OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-illusion-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Mislead OnSpellEffect"

	if staff_has(spell.caster) == 1:
		CL, mod = staff_stats (spell.caster, '10011')
		spell.caster_level = CL
		
	spell.duration = 1 * spell.caster_level
	target_item = spell.target_list[0]

	num_of_images = spell.caster_level

	#print "num of images=", num_of_images

	game.particles( 'sp-Mirror Image', target_item.obj )
	target_item.obj.condition_add_with_args( 'sp-Mirror Image', spell.id, spell.duration, num_of_images )
	#target_item.partsys_id = game.particles( 'sp-Mirror Image', target_item.obj )

#	spell.id = spell.id + 1

	target_item.obj.condition_add_with_args( 'sp-Improved Invisibility', spell.id, spell.duration, 0 )
	target.partsys_id = game.particles( 'sp-Improved Invisibility', target.obj )

#	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Mislead OnBeginRound"

def OnEndSpellCast( spell ):
	print "Mislead OnEndSpellCast"
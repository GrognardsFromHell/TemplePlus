from toee import *
from utilities import  * 

def OnBeginSpellCast( spell ):
	print "Resonance OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Resonance OnSpellEffect"
	target_item = spell.target_list[0]
	caster = spell.caster
	
	isValid = 0
	if caster.stat_level_get(stat_level_wizard) >= 1:
		caster.spells_pending_to_memorized(stat_level_wizard)
		isValid = 1
	if caster.stat_level_get(stat_level_bard) >= 1:
		caster.spells_cast_reset(stat_level_bard)
		isValid = 1
	if caster.stat_level_get(stat_level_sorcerer) >= 1:
		caster.spells_cast_reset(stat_level_sorcerer)
		isValid = 1
	
	if isValid:
		print "doing spells pending to memorized for spell caster"
		game.particles( 'sp-Read Magic', caster )
	else:
		game.particles( 'Fizzle', caster )
	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Resonance OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resonance OnEndSpellCast"

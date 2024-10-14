from toee import *
from utilities import  * 

def OnBeginSpellCast( spell ):
	print "Resonance OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Resonance OnSpellEffect"
	target_item = spell.target_list[0]
	if (spell.caster.stat_level_get(stat_level_wizard) >= 1 or spell.caster.stat_level_get(stat_level_bard) >= 1 or spell.caster.stat_level_get(stat_level_sorcerer) >= 1) and not (spell.caster.stat_level_get(stat_level_cleric) >= 1 or spell.caster.stat_level_get(stat_level_paladin) >= 1 or spell.caster.stat_level_get(stat_level_ranger) >= 1 or spell.caster.stat_level_get(stat_level_druid) >= 1):
		spell.caster.spells_pending_to_memorized()
		game.particles( 'sp-Read Magic', spell.caster )
	else:
		game.particles( 'Fizzle', spell.caster )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Resonance OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resonance OnEndSpellCast"

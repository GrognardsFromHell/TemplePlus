from toee import *

from Co8 import *

def OnBeginSpellCast( spell ):
	print "Reincarnation OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Reincarnation OnSpellEffect"

	spell.duration = 1
	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Raise Dead', spell.id, spell.duration, 0 )
	#target_item.partsys_id = game.particles( 'sp-Raise Dead', target_item.obj )

#	target_item.obj.stat_base_set(stat_race, (race_gnome))
	
	End_Spell(spell)
	
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Reincarnation OnBeginRound"

def OnEndSpellCast( spell ):
	print "Reincarnation OnEndSpellCast"
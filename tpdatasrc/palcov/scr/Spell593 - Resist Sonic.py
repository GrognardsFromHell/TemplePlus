from toee import *

def OnBeginSpellCast( spell ):
	print "Resist Sonic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Sonic OnSpellEffect"

	element_type = SONIC
	partsys_type = 'sp-Resist Elements-sonic'

	spell.duration = spell.caster_level*100
	if (spell.caster not in game.party) and (spell.caster.object_flags_get() & OF_INVULNERABLE == 0):
		spell.duration = 600*24*30  # 30 days

	target_item = spell.target_list[0]
	
	#target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, spell.duration, element_type )
	if target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration ):
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )


def OnBeginRound( spell ):
	print "Resist Sonic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Sonic OnEndSpellCast"
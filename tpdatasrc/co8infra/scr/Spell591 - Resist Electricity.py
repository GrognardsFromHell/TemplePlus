from toee import *

def OnBeginSpellCast( spell ):
	print "Resist Electricity OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Electricity OnSpellEffect"

	element_type = ELECTRICITY
	partsys_type = 'sp-Resist Elements-water'

	spell.duration = spell.caster_level*100

	target_item = spell.target_list[0]
	
	if target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type , spell.duration):
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )
	#target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration )
	#target_item.partsys_id = game.particles( partsys_type, target_item.obj )

def OnBeginRound( spell ):
	print "Resist Electricity OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Electricity OnEndSpellCast"
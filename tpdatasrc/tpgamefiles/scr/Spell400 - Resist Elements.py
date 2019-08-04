from toee import *

def OnBeginSpellCast( spell ):
	print "Resist Elements OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Resist Elements OnSpellEffect"

	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg == 1:
		element_type = ACID
		partsys_type = 'sp-Resist Elements-acid'
	elif spell_arg == 2:
		element_type = COLD
		partsys_type = 'sp-Resist Elements-cold'
	elif spell_arg == 3:
		element_type = ELECTRICITY
		partsys_type = 'sp-Resist Elements-water'
	elif spell_arg == 4:
		element_type = FIRE
		partsys_type = 'sp-Resist Elements-fire'
	elif spell_arg == 5:
		element_type = SONIC
		partsys_type = 'sp-Resist Elements-sonic'

	spell.duration = 100 * spell.caster_level

	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Resist Elements', spell.id, element_type, spell.duration )
	target_item.partsys_id = game.particles( partsys_type, target_item.obj )

	#spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Resist Elements OnBeginRound"

def OnEndSpellCast( spell ):
	print "Resist Elements OnEndSpellCast"
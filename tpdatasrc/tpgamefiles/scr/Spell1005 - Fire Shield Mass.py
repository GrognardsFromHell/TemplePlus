from toee import *

def OnBeginSpellCast( spell ):
	print "Fire Shield Mass OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Fire Shield Mass OnSpellEffect"

	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg == 1:
		element_type = COLD
		partsys_type = 'sp-Fire Shield-Cold'
	elif spell_arg == 2:
		element_type = FIRE
		partsys_type = 'sp-Fire Shield-Warm'

	spell.duration = 1 * spell.caster_level

	for target_item in spell.target_list:
		target_item.obj.condition_add_with_args( 'sp-Fire Shield', spell.id, spell.duration, element_type )
		target_item.partsys_id = game.particles( partsys_type, target_item.obj )

def OnBeginRound( spell ):
	print "Fire Shield Mass OnBeginRound"

def OnEndSpellCast( spell ):
	print "Fire Shield Mass OnEndSpellCast"
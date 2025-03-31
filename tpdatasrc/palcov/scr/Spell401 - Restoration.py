from toee import *

def OnBeginSpellCast( spell ):
	print "Restoration OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Restoration OnSpellEffect"

	spell.duration = 0
	target_item = spell.target_list[0]

	# get the ability type (from radial menu, -1 to offset index and D20Strength == 0)
	ability_type = (spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING ) - 1)

	## Solves Radial menu problem for Wands/NPCs
	if ability_type != 0 and ability_type != 1 and ability_type != 2 and ability_type != 3 and ability_type != 4 and ability_type != 5:
		ability_type = game.random_range(1,6)
		ability_type = ability_type - 1

	if target_item.obj.is_friendly( spell.caster ) or spell.caster.name == 14656:  # Mabon

		target_item.obj.condition_add_with_args( 'sp-Restoration', spell.id, spell.duration, ability_type )
		target_item.partsys_id = game.particles( 'sp-Restoration', target_item.obj )

	elif not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

		# saving throw unsuccesful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		target_item.obj.condition_add_with_args( 'sp-Restoration', spell.id, spell.duration, ability_type )
		target_item.partsys_id = game.particles( 'sp-Restoration', target_item.obj )

	else:

		# saving throw successful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

		game.particles( 'Fizzle', target_item.obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Restoration OnBeginRound"

def OnEndSpellCast( spell ):
	print "Restoration OnEndSpellCast"
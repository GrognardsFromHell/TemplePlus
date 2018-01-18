from toee import *

def OnBeginSpellCast( spell ):
	print "Wall of Fire OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Wall of Fire OnSpellEffect"

	remove_list = []

	damage_dice = dice_new( '2d6' )
	damage_dice.bonus = min( 1 * spell.caster_level, 20 )

	spell.duration = spell.caster_level

	#game.particles( 'sp-Lightning Bolt', spell.target_loc )
	#game.pfx_lightning_bolt( spell.caster, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y, spell.target_loc_off_z )

	for target_item in spell.target_list:
		target_item.obj.spell_damage(spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
		remove_list.append( target_item.obj )


	# todo: spawn spell object and give it a wall-of-fire condition
	spell_obj = game.obj_create(OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )
	wall_end_point = spell.spell_get_picker_end_point()
	spell_obj_2 = game.obj_create(OBJECT_SPELL_GENERIC, wall_end_point.get_location(), wall_end_point.off_x, wall_end_point.off_y)
	spell_obj.turn_towards(spell_obj_2)
	spell_obj.rotation -= 3.1415 / 2
	x = 0
	y = 0 # todo set to be the floating point offset of the spell_obj_2

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative(caster_init_value)

	# put sp-Entangle condition on obj
	spell_partsys_id = game.particles('sp-Wall of Fire2', spell_obj)
	game.particles('sp-Wall of Fire2', spell_obj_2)
	spell_obj_2.turn_towards(spell_obj)
	spell_obj_2.rotation -= 3.1415 / 2
	#rot = spell_obj.rotation
	rot = 0
	spell_obj.condition_add_with_args('sp-Wall of Fire', spell.id, spell.duration, 0, spell_partsys_id, rot , x, y )

	spell.target_list.remove_list( remove_list )

def OnBeginRound( spell ):
	print "Wall of Fire OnBeginRound"

def OnEndSpellCast( spell ):
	print "Wall of Fire OnEndSpellCast"
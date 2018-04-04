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
	undead_dice = dice_new('4d6')
	undead_dice.bonus = min( 2 * spell.caster_level, 40 )
	spell.duration = spell.caster_level

	for target_item in spell.target_list:
		#if target_item.obj.is_category_type( mc_type_undead ):
		#	target_item.obj.spell_damage(spell.caster, D20DT_FIRE, undead_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
		#else:
		#	target_item.obj.spell_damage(spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id)
		remove_list.append( target_item.obj )

	# Create spell object at start point
	spell_obj = game.obj_create(OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )
	spell_obj.move(spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y) # set precise position (in case it gets relocated by the engine)
	x,y = location_to_axis(spell_obj.location)
	#print "Src loc x,y: " + str(x) + " " + str(y)
	#print "Src loc off_x: " + str(spell_obj.off_x)
	#print "Src loc off_y: " + str(spell_obj.off_y)
	
	# Create 2nd spell object at wall end point
	wall_end_point = spell.spell_get_picker_end_point()
	
	x,y = location_to_axis(wall_end_point.get_location())
	#print "End loc x,y: " + str(x) + " " + str(y)
	#print "End loc off_x: " + str(wall_end_point.off_x)
	#print "End loc off_y: " + str(wall_end_point.off_y)
	spell_obj_endpt = game.obj_create(OBJECT_SPELL_GENERIC, wall_end_point.get_location(), wall_end_point.off_x, wall_end_point.off_y)
	spell_obj_endpt.move(wall_end_point.get_location(), wall_end_point.off_x, wall_end_point.off_y) # set precise position (in case it gets relocated by the engine)
	# rotate the spell object towards the endpoint
	spell_obj.turn_towards(spell_obj_endpt)
	wall_angle_mrad = 1000*spell_obj.rotation
	#spell_obj.rotation -= 3.1415 / 2
	wall_length_mft = 1000*spell_obj.location_full.distance_to(wall_end_point)
	#print "Wall angle(mrad): " + str(wall_angle_mrad)
	#print "Wall length(mft): " + str(wall_length_mft)
	spell_obj_endpt.destroy()

	# add to d20initiative (so the next time goes off in the correct sequence)
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative(caster_init_value)

	# put sp-Wall of Fire condition on obj
	spell_obj.condition_add_with_args('sp-Wall of Fire', spell.id, spell.duration, 0, int(wall_angle_mrad) ,  int(wall_length_mft))

	spell.target_list.remove_list( remove_list )

def OnBeginRound( spell ):
	print "Wall of Fire OnBeginRound"

def OnEndSpellCast( spell ):
	print "Wall of Fire OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Shout OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Shout OnSpellEffect"

	remove_list = []

	damage_dice = dice_new( '5d6' )
	duration_dice = dice_new( '2d6' )
	spell.duration = duration_dice.roll()
	earth_dam = dice_new ( '1d6' )
	earth_dam.number = min( 15, spell.caster_level )

	game.particles( 'sp-Shout', spell.caster )

	# get all targets in a 25ft + 2ft/level cone (60')

	npc = spell.caster			##  added so NPC's can target Shout

	#### Caster is NOT in game party
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
#		range = 25 + 2 * int(spell.caster_level/2)
		range = 30
		target_list = list(game.obj_list_cone( spell.caster, OLC_CRITTERS, range, -30, 90 ))
		target_list.remove(spell.caster)
		for obj in target_list:
			if not obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
				# saving throw unsuccessful
				obj.spell_damage( spell.caster, D20DT_SONIC, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
				obj.condition_add_with_args( 'sp-Shout', spell.id, spell.duration, 0 )
#				obj.partsys_id = game.particles( 'sp-Shout-Hit', obj )
				game.particles( 'sp-Shout-Hit', obj )
			else:
				# saving throw successful, apply half damage
				obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
				obj.spell_damage_with_reduction( spell.caster, D20DT_SONIC, damage_dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
#				obj.partsys_id = game.particles( 'sp-Shout-Hit', obj )
				game.particles( 'sp-Shout-Hit', obj )
				game.particles( 'Fizzle', obj )


	#### Caster is in game party
	if npc.type == obj_t_pc or npc.leader_get() != OBJ_HANDLE_NULL:
		for target_item in spell.target_list:

			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
				# saving throw unsuccessful
				target_item.obj.spell_damage( spell.caster, D20DT_SONIC, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
	
				target_item.obj.condition_add_with_args( 'sp-Shout', spell.id, spell.duration, 0 )
				target_item.partsys_id = game.particles( 'sp-Shout-Hit', target_item.obj )
	
			else:
	
				# saving throw successful, apply half damage
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
				target_item.obj.spell_damage_with_reduction( spell.caster, D20DT_SONIC, damage_dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
				target_item.partsys_id = game.particles( 'sp-Shout-Hit', target_item.obj )
	
				game.particles( 'Fizzle', target_item.obj )
				remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Shout OnBeginRound"

def OnEndSpellCast( spell ):
	print "Shout OnEndSpellCast"
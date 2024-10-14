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

	game.particles( 'sp-Shout', spell.caster )

	# When the spell is cast by an NPC, it creates a target list with a 360 cone.
	# Changing the angle to 90 in the rules file does not fix this.
	# So, the list needs to be trimmed down to 90 degrees here. marc.

	if spell.caster.leader_get() == OBJ_HANDLE_NULL and spell.caster.type != obj_t_pc:
		valid_target_list = game.obj_list_cone (spell.caster, OLC_CRITTERS, 30, -45, 90)

	# get all targets in a 25ft + 2ft/level cone (60')
	for target_item in spell.target_list:

		# ignore targets not in the 90 foot cone for NPC casters, marc
		if spell.caster.leader_get() == OBJ_HANDLE_NULL and spell.caster.type != obj_t_pc:
			if target_item.obj not in valid_target_list:
				#float_num(target_item.obj,777)
				continue

		if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):

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
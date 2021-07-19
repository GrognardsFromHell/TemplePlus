from toee import *

def OnBeginSpellCast( spell ):
	print "Ray of Exhaustion OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect( spell ):
	print "Ray of Exhaustion OnSpellEffect"

def OnBeginRound( spell ):
	print "Ray of Exhaustion OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Ray of Exhaustion OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Ray of Enfeeblement', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Ray of Exhaustion OnEndProjectile"

	spell.duration = 10 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	if spell.caster.perform_touch_attack( target_item.obj ) & D20CAF_HIT:
		wrongType = target_item.obj.is_category_type(mc_type_undead) or target_item.obj.is_category_type(mc_type_construct) or target_item.obj.is_category_type(mc_type_ooze) or target_item.obj.is_category_type(mc_type_plant)
		if wrongType:
			target_item.obj.float_text_line("Fatigue Immunity")
		else:
			hasExhaust = target_item.obj.d20_query("Exhausted")
			hasFatigue = target_item.obj.d20_query("Fatigued")
			if hasExhaust:
				target_item.obj.d20_send_signal("Add Exhaustion", spell.duration) #Upgrage to exhaustion regardless of save
			if hasFatigue:
				target_item.obj.d20_send_signal("Add Fatigue", spell.duration) #Upgrage to exhaustion regardless of save
			else:
				# hit
				if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
					# saving throw successful
					target_item.obj.condition_add_with_args("FatigueExhaust", 0, spell.duration, 0, 1, 0, 0)
				else:
					# saving throw unsuccessful
					target_item.obj.condition_add_with_args("FatigueExhaust", 0, spell.duration, spell.duration, 1, 0, 0)
	else:
		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
		game.particles( 'Fizzle', target_item.obj )
	
	spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Ray of Exhaustion OnEndSpellCast"
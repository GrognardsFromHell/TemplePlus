from toee import *

def OnBeginSpellCast( spell ):
	print "Melf's Acid Arrow OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )
	#spell.variables = [0,0]

def OnSpellEffect( spell ):
	print "Melf's Acid Arrow OnSpellEffect"

def OnBeginRound( spell ):
	print "Melf's Acid Arrow OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Melf's Acid Arrow OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Melfs Acid Arrow Projectile', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Melf's Acid Arrow OnEndProjectile"

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	# calculate spell.duration
	dur = spell.caster_level / 3
	if dur < 0:
		dur = 0
	elif dur > 20:
		dur = 20
	spell.duration = dur
	if (spell.caster_level >= 3) and (spell.caster_level <= 5):
		spell.duration = 2 - 1
	elif (spell.caster_level >=6) and (spell.caster_level <= 8):
		spell.duration = 3 - 1
	elif (spell.caster_level >=9) and (spell.caster_level <= 11):
		spell.duration = 4 - 1
	elif (spell.caster_level >=12) and (spell.caster_level <= 14):
		spell.duration = 5 - 1
	elif (spell.caster_level >=15) and (spell.caster_level <= 17):
		spell.duration = 6 - 1
	elif (spell.caster_level >=18) and (spell.caster_level <= 20):
		spell.duration = 7 - 1
	else:
		spell.duration = 1 - 1

	target = spell.target_list[0]

	if not (target.obj == spell.caster):

		attack_successful = spell.caster.perform_touch_attack( target.obj )
	
		# perform ranged touch attack
		if attack_successful & D20CAF_HIT:
	
			# hit
			if attack_successful & D20CAF_CRITICAL:
				target.obj.condition_add_with_args( 'sp-Melfs Acid Arrow', spell.id, spell.duration, 1 )
			else:
				target.obj.condition_add_with_args( 'sp-Melfs Acid Arrow', spell.id, spell.duration, 0 )
			target.partsys_id = game.particles( 'sp-Melfs Acid Arrow Projectile Hit', target.obj )
		else:
	
			# missed
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
	
			game.particles( 'Fizzle', target.obj )
			spell.target_list.remove_target( target.obj )

	else:

		print "creating acid arrows not supported yet!"

	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Melf's Acid Arrow OnEndSpellCast"
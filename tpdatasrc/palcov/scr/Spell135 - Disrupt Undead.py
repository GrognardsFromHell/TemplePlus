from toee import *

def OnBeginSpellCast( spell ):
	print "Disrupt Undead OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Disrupt Undead OnSpellEffect"

def OnBeginRound( spell ):
	print "Disrupt Undead OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Disrupt Undead OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Disrupt Undead-proj', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Disrupt Undead-proj', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Disrupt Undead OnEndProjectile"

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target = spell.target_list[0]

	if target.obj.is_category_type( mc_type_undead ):
	
		# perform ranged touch attack
		attack_successful = spell.caster.perform_touch_attack( target.obj )
		if attack_successful & D20CAF_HIT:

			damage_dice = dice_new( '1d6' )
	
			# hit
			target.obj.spell_damage_weaponlike( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, attack_successful, index_of_target )
			target.partsys_id = game.particles( 'sp-Disrupt Undead-hit', target.obj )
		else:
	
			# missed
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
	
			game.particles( 'Fizzle', target.obj )

	else:

		# not undead!
		target.obj.float_mesfile_line( 'mes\\spell.mes', 31008 )

		game.particles( 'Fizzle', target.obj )

	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Disrupt Undead OnEndSpellCast"
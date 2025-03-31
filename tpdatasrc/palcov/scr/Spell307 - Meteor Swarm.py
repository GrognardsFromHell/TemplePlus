from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Meteor Swarm OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Meteor Swarm OnSpellEffect"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Meteor Swarm OnBeginProjectile"

	projectiles = 4
	if index_of_target < projectiles:
		projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Spheres of Fire-proj', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Meteor Swarm OnEndProjectile"

	dam = dice_new( '2d6' )
	dam2 = dice_new( '6d6' )

	projectiles = 4
	if index_of_target < projectiles:
		spell.duration = 0

		game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

		target_item = spell.target_list[ index_of_target ]

		return_val = spell.caster.perform_touch_attack( target_item.obj )
		xx,yy = location_to_axis(target_item.obj.location)
		if target_item.obj.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
			target_item.obj.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1)
			game.particles( 'swirled gas', target_item.obj )
			game.sound(7581,1)
			game.sound(7581,1)
		else:
			if (return_val & D20CAF_HIT):
				# hit target
				if index_of_target > 0:
					return_val |= D20CAF_NO_PRECISION_DAMAGE
				game.particles( 'sp-Spheres of Fire-hit', target_item.obj )
				target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_BLUDGEONING, dam, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, return_val, index_of_target )
				target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_FIRE, dam2, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, return_val, index_of_target )

			else:
				# miss target
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
				game.particles( 'Fizzle', target_item.obj )

				if target_item.obj.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam2, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
					# saving throw successful
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
				else:
					# saving throw unsuccessful
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			game.particles( 'sp-Fireball-Hit', target_item.obj )

			for critter in game.obj_list_cone( target_item.obj, OLC_CRITTERS, 40, -180, 360 ):
				if (critter != target_item.obj) and (critter.d20_query(Q_Dead) == 0):
					xx,yy = location_to_axis(critter.location)
					if critter.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
						critter.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1)
						game.particles( 'swirled gas', critter )
						game.sound(7581,1)
						game.sound(7581,1)
					else:
						game.particles( 'hit-FIRE-burst', critter )
						if critter.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam2, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
							# saving throw successful
							critter.float_mesfile_line( 'mes\\spell.mes', 30001 )
						else:
							# saving throw unsuccessful
							critter.float_mesfile_line( 'mes\\spell.mes', 30002 )

		spell.num_of_projectiles = spell.num_of_projectiles - 1

	if ( spell.num_of_projectiles <= 0 ):
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Meteor Swarm OnEndSpellCast"
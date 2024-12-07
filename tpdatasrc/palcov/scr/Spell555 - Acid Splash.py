from toee import *

def OnBeginSpellCast( spell ):
	print "Acid Splash OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Acid Splash OnSpellEffect"

def OnBeginRound( spell ):
	print "Acid Splash OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Acid Splash OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Acid Splash', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Melfs Acid Arrow Projectile', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Acid Splash OnEndProjectile"

	damage_dice = dice_new( '1d3' )

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	return_val = spell.caster.perform_touch_attack( target_item.obj )
	if return_val & D20CAF_HIT:

		game.particles( 'sp-Acid Splash-Hit', target_item.obj )

		# hit
		target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_ACID, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, return_val, index_of_target )
	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Acid Splash OnEndSpellCast"
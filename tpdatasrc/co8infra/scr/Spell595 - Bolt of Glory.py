from toee import *

def OnBeginSpellCast( spell ):
	print "Bolt of Glory OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Bolt of Glory OnSpellEffect"

def OnBeginRound( spell ):
	print "Bolt of Glory OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Bolt of Glory OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Frost', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Searing Light', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Bolt of Glory OnEndProjectile"
	
	target_item = spell.target_list[0]
	damage_dice = dice_new( '1d6' )
	if (target_item.obj.is_category_type( mc_type_outsider ) and (alignment & ALIGNMENT_EVIL)) or target_item.obj.is_category_type( mc_type_undead ):
		damage_dice.num = min(15, spell.caster_level)
	else:
		damage_dice.num = min(7, spell.caster_level/2)

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	return_val = spell.caster.perform_touch_attack( target_item.obj )
	if return_val & D20CAF_HIT:

		game.particles( 'sp-Searing Light-Hit', target_item.obj )

		# hit
		if (target_item.obj.is_category_type( mc_type_outsider ) and (alignment & ALIGNMENT_GOOD)):
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 32060 )
			game.particles( 'Fizzle', target_item.obj )
		else:
			target_item.obj.spell_damage_weaponlike( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id,return_val , index_of_target )

	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Bolt of Glory OnEndSpellCast"

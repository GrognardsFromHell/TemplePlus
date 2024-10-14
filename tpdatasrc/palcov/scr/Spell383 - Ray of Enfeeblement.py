from toee import *

def OnBeginSpellCast( spell ):
	print "Ray of Enfeeblement OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Ray of Enfeeblement OnSpellEffect"

def OnBeginRound( spell ):
	print "Ray of Enfeeblement OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Ray of Enfeeblement OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Enfeeblement', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Ray of Enfeeblement', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Ray of Enfeeblement OnEndProjectile"

	dice = dice_new( '1d6' )
	dam_amount = dice.roll()
	dam_amount += min( 5, spell.caster_level / 2 )
	print "amount=", dam_amount

	spell.duration = 10 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	# hit
	if spell.caster.perform_touch_attack( target_item.obj ) >= 1:

		# Adjust damage if the target's strength would drop below 1
		target_strength = target_item.obj.stat_level_get(stat_strength)
		if target_strength > 1:
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 20022, tf_red )
			if dam_amount >= target_strength:
				dam_amount = target_strength - 1
		else:
			dam_amount = 0

		target_item.obj.condition_add_with_args( 'sp-Ray of Enfeeblement', spell.id, spell.duration, dam_amount )
		target_item.partsys_id = game.particles( 'sp-Ray of Enfeeblement-Hit', target_item.obj )

	# missed
	else:
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	# Restore dexterity
	if spell.caster.has_feat(feat_weapon_focus_ray):
		spell.caster.stat_base_set(stat_dexterity, dex)

	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Ray of Enfeeblement OnEndSpellCast"
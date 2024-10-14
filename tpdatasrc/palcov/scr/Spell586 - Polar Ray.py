from toee import *

def OnBeginSpellCast( spell ):
	print "Polar Ray OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Polar Ray OnSpellEffect"

def OnBeginRound( spell ):
	print "Polar Ray OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Polar Ray OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Frost', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Ray of Frost', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Polar Ray OnEndProjectile"

	if spell.caster.name == 14714:  # Shadow Wizard
		spell.caster_level = 20
	if spell.caster.name == 14968:  # Lich
		spell.caster_level = 20

	damage_dice = dice_new( '1d6' )
	damage_dice.num = min (25, spell.caster_level)

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	return_val = spell.caster.perform_touch_attack( target_item.obj )

	# hit
	if return_val == 1:
		game.particles( 'sp-Ray of Frost-Hit', target_item.obj )
		target_item.obj.spell_damage( spell.caster, D20DT_COLD, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	# critical hit
	elif return_val == 2:
		game.particles( 'sp-Ray of Frost-Hit', target_item.obj )
		damage_dice.num = damage_dice.num * 2
		target_item.obj.spell_damage( spell.caster, D20DT_COLD, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	# missed
	else:
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
		game.particles( 'Fizzle', target_item.obj )

	# Restore dexterity
	if spell.caster.has_feat(feat_weapon_focus_ray):
		spell.caster.stat_base_set(stat_dexterity, dex)

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Polar Ray OnEndSpellCast"

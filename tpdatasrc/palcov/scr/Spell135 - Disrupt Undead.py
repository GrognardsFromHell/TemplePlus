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

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	if target.obj.is_category_type( mc_type_undead ):
	
		# perform ranged touch attack
		attack_successful = spell.caster.perform_touch_attack( target.obj )

		# hit
		if attack_successful == 1:
			damage_dice = dice_new( '1d6' )
			target.obj.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			target.partsys_id = game.particles( 'sp-Disrupt Undead-hit', target.obj )

		#critical hit
		elif attack_successful == 2:
			damage_dice = dice_new( '2d6' )
			target.obj.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			target.partsys_id = game.particles( 'sp-Disrupt Undead-hit', target.obj )
	
		# missed
		else:
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
			game.particles( 'Fizzle', target.obj )

	else:

		# not undead!
		target.obj.float_mesfile_line( 'mes\\spell.mes', 31008 )
		game.particles( 'Fizzle', target.obj )

	# Restore dexterity
	if spell.caster.has_feat(feat_weapon_focus_ray):
		spell.caster.stat_base_set(stat_dexterity, dex)

	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Disrupt Undead OnEndSpellCast"
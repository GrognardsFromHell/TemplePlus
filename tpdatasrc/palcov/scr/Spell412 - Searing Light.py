from toee import *

def OnBeginSpellCast( spell ):
	print "Searing Light OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Searing Light OnSpellEffect"

def OnBeginRound( spell ):
	print "Searing Light OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Searing Light OnBeginProjectile"
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Searing Light', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Searing Light OnEndProjectile"

	damage_dice = dice_new('1d8')
	damage_dice.number = min (5, int(spell.caster_level*0.5) )

	spell.duration = 0

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	if target_item.obj.is_category_type(mc_type_undead):
		damage_dice.number = min (10, spell.caster_level)
		damage_dice.size = 6
		if target_item.obj.name in (14328,14849,14986):  # bodak, wraith, dread wraith
			damage_dice.size = 8
	elif target_item.obj.is_category_type(mc_type_construct):
		damage_dice.size = 6

	if spell.caster.name in (14396,14888):  # lantern archon, marc 5/8/16
		damage_dice = dice_new('2d6')

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	attack_successful = spell.caster.perform_touch_attack( target_item.obj )

	# hit
	if attack_successful == 1:
		game.particles( 'sp-Searing Light-Hit', target_item.obj )
		target_item.obj.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	# critical hit
	elif attack_successful == 2:
		game.particles( 'sp-Searing Light-Hit', target_item.obj )
		damage_dice.number = damage_dice.number * 2
		target_item.obj.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

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
	print "Searing Light OnEndSpellCast"
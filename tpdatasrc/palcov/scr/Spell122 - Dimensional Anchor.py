from toee import *

def OnBeginSpellCast( spell ):
	print "Dimensional Anchor OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Dimensional Anchor OnSpellEffect"

def OnBeginRound( spell ):
	print "Dimensional Anchor OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Dimensional Anchor OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Dimensional Anchor-proj', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Dimensional Anchor OnEndProjectile"

	spell.duration = 60 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	# hit
	if spell.caster.perform_touch_attack( target_item.obj ) >= 1:
		target_item.obj.condition_add_with_args( 'sp-Dimensional Anchor', spell.id, spell.duration, 0 )
		target_item.partsys_id = game.particles( 'sp-Dimensional Anchor', target_item.obj )

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
	print "Dimensional Anchor OnEndSpellCast"
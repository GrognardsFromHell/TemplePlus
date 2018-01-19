from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Scorching Ray OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Scorching Ray OnSpellEffect"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Scorching Ray OnBeginProjectile"

	projectiles = min(3, (spell.caster_level + 1) / 4)
	if index_of_target < projectiles:
		projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Scorching Ray', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Scorching Ray OnEndProjectile"

	####################################################
	# WF Ray fix added by Shiningted (& two lines below)
	####################################################

	has_it = 0
	x = 0
	y = 0

	if spell.caster.has_feat(feat_weapon_focus_ray):
		# game.particles( "sp-summon monster I", game.party[0] )
		has_it = 1
		x = spell.caster.stat_base_get(stat_dexterity)
		y = x + 2
		if spell.caster.has_feat(feat_greater_weapon_focus_ray):
			y = y + 2
		spell.caster.stat_base_set(stat_dexterity, y)

	####################################################

	projectiles = min(3, (spell.caster_level + 1) / 4)
	if index_of_target < projectiles:
		spell.duration = 0

		game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

		target = spell.target_list[ index_of_target ]

		return_val = spell.caster.perform_touch_attack( target.obj )
		
		xx,yy = location_to_axis(target.obj.location)
		if target.obj.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
			target.obj.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1)
			game.particles( 'swirled gas', target.obj )
			game.sound(7581,1)
			game.sound(7581,1)
		elif return_val & D20CAF_HIT:
			if index_of_target > 0:
				return_val |= D20CAF_NO_PRECISION_DAMAGE
			damage_dice = dice_new( '4d6' )
			game.particles( 'sp-Scorching Ray-Hit', target.obj )
			target.obj.spell_damage_weaponlike( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, return_val, index_of_target )
		else:

			# missed
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

			game.particles( 'Fizzle', target.obj )

	if has_it == 1:
		spell.caster.stat_base_set(stat_dexterity, x)

	spell.num_of_projectiles = spell.num_of_projectiles - 1
	if ( spell.num_of_projectiles == 0 ):
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Scorching Ray OnEndSpellCast"
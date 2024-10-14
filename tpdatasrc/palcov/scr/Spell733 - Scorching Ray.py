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

	dbug("\nOnBeginProjectile\n  spell.target_list",spell.target_list,"scorching_ray")
	dbug("  spell.num_of_projectiles",spell.num_of_projectiles,"scorching_ray")
	dbug("  projectile",projectile,"scorching_ray")
	dbug("  index_of_target",index_of_target,"scorching_ray")

	projectiles = min( 3, (spell.caster_level + 1) / 4 )

	if spell.caster.name == 14340:  # efreeti
		spell.dc = 14               # 10 + 2 + 2 (charisma)
		spell.caster_level = 12
		projectiles = 1

	if index_of_target < projectiles:
		projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Scorching Ray', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Scorching Ray OnEndProjectile"

	dbug("\nOnEndProjectile\n  spell.target_list",spell.target_list,"scorching_ray")
	dbug("  spell.num_of_projectiles",spell.num_of_projectiles,"scorching_ray")
	dbug("  projectile",projectile,"scorching_ray")
	dbug("  index_of_target",index_of_target,"scorching_ray")

	# Weapon Focus Ray, fix added by Shiningted
	if spell.caster.has_feat(feat_weapon_focus_ray):
		dex = spell.caster.stat_base_get(stat_dexterity)
		dex_temp = dex + 2 + spell.caster.has_feat(feat_greater_weapon_focus_ray)*2
		spell.caster.stat_base_set (stat_dexterity, dex_temp)

	projectiles = min( 3, (spell.caster_level + 1) / 4 )

	if spell.caster.name == 14340:	# efreeti
		spell.dc = 14				# 10 + 2 + 2 (charisma)
		spell.caster_level = 12
		projectiles = 1

	if index_of_target < projectiles:
		spell.duration = 0
		game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

		# NPC may get extra rays, marc
		extra = 0
		if not spell.caster in game.party: 
			if spell.caster.name in [14893,14792,14793]:  # 1 extra ray (orc warlock, W7, W9), 
				extra = 1
			elif spell.caster.name in [14537]:            # 2 extra rays, (thormund)
				extra = 2

		target = spell.target_list[ index_of_target ]
		return_val = spell.caster.perform_touch_attack( target.obj )

		# hit
		if return_val == 1:
			damage_dice = dice_new( '4d6' )
			game.particles( 'sp-Scorching Ray-Hit', target.obj )
			target.obj.spell_damage( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			for x in range(0,extra):  # npc extra rays, marc
				target.obj.spell_damage( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

		# critical hit
		elif return_val == 2:
			damage_dice = dice_new( '8d6' )
			game.particles( 'sp-Scorching Ray-Hit', target.obj )
			target.obj.spell_damage( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			for x in range(0,extra):  # npc extra rays, marc
				target.obj.spell_damage( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

		# missed
		else:
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
			game.particles( 'Fizzle', target.obj )

		spell.num_of_projectiles = spell.num_of_projectiles - 1

	# Restore dexterity
	if spell.caster.has_feat(feat_weapon_focus_ray):
		spell.caster.stat_base_set(stat_dexterity, dex)

	if ( spell.num_of_projectiles <= 0 ):
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Scorching Ray OnEndSpellCast"
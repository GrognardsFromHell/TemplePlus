from toee import *

def OnBeginSpellCast( spell ):
	print "Ray of Cluminess OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Ray of Clumsiness OnSpellEffect"

def OnBeginRound( spell ):
	print "Ray of Clumsiness OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Ray of Clumsiness OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Enfeeblement', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Ray of Enfeeblement', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Ray of Clumsiness OnEndProjectile"
	target_item = spell.target_list[0]

	dice = dice_new( '1d6' )
	dam_amount = dice.roll()
	dam_amount += min(5, spell.caster_level / 2)	
	if dam_amount >= target_item.obj.stat_level_get(stat_dexterity):
		dam_amount = target_item.obj.stat_level_get(stat_dexterity) -1 
	dam_amount = -dam_amount 
	print "amount=", dam_amount

	spell.duration = 10 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

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

	if spell.caster.perform_touch_attack( target_item.obj ) & D20CAF_HIT:

		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 20022, tf_red )
		target_item.obj.condition_add_with_args( 'sp-Cats Grace', spell.id, spell.duration, dam_amount )
		target_item.partsys_id = game.particles( 'sp-Ray of Enfeeblement-Hit', target_item.obj )

	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	if has_it == 1:
		spell.caster.stat_base_set(stat_dexterity, x)

	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Ray of Clumsiness OnEndSpellCast"
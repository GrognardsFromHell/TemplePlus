from toee import *
import tpdp

def OnBeginSpellCast( spell ):
	print "Ray of Enfeeblement OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def OnSpellEffect( spell ):
	print "Ray of Enfeeblement OnSpellEffect"

def OnBeginRound( spell ):
	print "Ray of Enfeeblement OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Ray of Enfeeblement OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Enfeeblement', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Ray of Enfeeblement', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Ray of Enfeeblement OnEndProjectile"
	
	dam_bonus = min( 5, spell.caster_level / 2 )
	dam_amount = spell.roll_dice_with_metamagic(1, 6, dam_bonus)	

	
	print "amount=", dam_amount

	spell.duration = 10 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )
	target_item = spell.target_list[0]

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

		# HTN - 3.5, no fortitude save
		# hit
		#if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
			# saving throw successful
			#target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
			#game.particles( 'Fizzle', target_item.obj )
			#spell.target_list.remove_target( target_item.obj )
		#else:
			# saving throw unsuccessful
			#target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		# check target strength and adjust damage if the target's strength drops below 1
		target_strength = target_item.obj.stat_level_get(stat_strength)
		if target_strength > 1:
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 20022, tf_red )
			if target_strength - dam_amount < 1:
				z = 0
				while z < target_strength:
					if target_strength - z == 1:
						dam_amount = z
					z = z + 1
		else:
			dam_amount = 0

		target_item.obj.condition_add_with_args( 'sp-Ray of Enfeeblement', spell.id, spell.duration, dam_amount )
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
	print "Ray of Enfeeblement OnEndSpellCast"
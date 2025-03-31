from toee import *
from casters import staff_has, staff_stats
from Co8 import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Disintegrate OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Disintegrate OnSpellEffect"

def OnBeginRound( spell ):
	print "Disintegrate OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Disintegrate OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Ray of Frost', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Disintegrate', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Disintegrate OnEndProjectile"

	if staff_has(spell.caster) == 1:
		CL, mod = staff_stats (spell.caster, '00011')
		spell.caster_level = CL
		spell.dc = 10 + 6 + mod
	
	damage_dice = dice_new( "1d6" )
	damage_dice.num = min( 40, spell.caster_level * 2 )
	spell.duration = 0

	is_immune_to_crit = 0
	changed_con = 0

	target_item = spell.target_list[0]
	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	return_val = spell.caster.perform_touch_attack( target_item.obj )

	# hit
	if return_val & D20CAF_HIT:

		game.particles( 'sp-Disintegrate-Hit', target_item.obj )

		# Spiritual Weapon
		if target_item.obj.name in (14629,14621,14604) and not is_spell_flag_set(target_item.obj, OSF_IS_FLAMING_SPHERE):
			if target_item.obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) == 1:
				target_item.obj.condition_add_with_args( 'sp-Break Enchantment', spell.id, spell.duration, 0 )
				game.particles( 'sp-Otilukes Resilient Sphere-END', target_item.obj.location )
			else:
				game.particles( 'sp-Stoneskin', target_item.obj.location )
				target_item.obj.destroy()

		else:

			if target_item.obj.is_category_type( mc_type_construct ) or target_item.obj.is_category_type( mc_type_undead ):
				if target_item.obj.stat_base_get(stat_constitution) < 0:
					target_item.obj.stat_base_set(stat_constitution, 10)
					changed_con = 1
				is_immune_to_crit = 1
			elif target_item.obj.is_category_type( mc_type_plant ) or target_item.obj.is_category_type( mc_type_ooze ) or target_item.obj.is_category_type( mc_type_elemental ):
				is_immune_to_crit = 1
			elif return_val == 2:
				damage_dice.num = damage_dice.num * 2

			if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
				damage_dice.num = 5
			#elif return_val == 2:
			#	damage_dice.num = damage_dice.num * 2 # handled internally now
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			target_item.obj.spell_damage( spell.caster, D20DT_FORCE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )			
			if target_item.obj.stat_level_get(stat_hp_current) < 1:
				target_item.obj.critter_kill_by_effect()
				# Don't Animate Dead on party members, only NPCs, it messes up spell scripting, marc 
				if target_item.obj.type != obj_t_pc and target_item.obj.leader_get() == OBJ_HANDLE_NULL:
					target_item.obj.condition_add_with_args( 'sp-Animate Dead', spell.id, spell.duration, 3 )
				game.particles( 'sp-Stoneskin', target_item.obj )

			# check for Otiluke's Resilient Sphere
			if target_item.obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) == 1:
				target_item.obj.condition_add_with_args( 'sp-Break Enchantment', spell.id, spell.duration, 0 )
				game.particles( 'sp-Otilukes Resilient Sphere-END', target_item.obj )

	# missed
	else:
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )
		game.particles( 'Fizzle', target_item.obj )

	if changed_con == 1:
		target_item.obj.stat_base_set(stat_constitution, -1)

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Disintegrate OnEndSpellCast"
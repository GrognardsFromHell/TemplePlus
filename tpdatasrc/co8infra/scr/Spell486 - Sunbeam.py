from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Sunbeam OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Sunbeam OnSpellEffect"

	if spell.caster_level >= 18:
		bonus = 6
	elif spell.caster_level >= 15:
		bonus = 5
	elif spell.caster_level >= 12:
		bonus = 4
	elif spell.caster_level >= 9:
		bonus = 3
	elif spell.caster_level >= 6:
		bonus = 2
	else:
		bonus = 1

	spell.duration = 1 * bonus

	target = spell.target_list[0]

	target.obj.condition_add_with_args( 'sp-Produce Flame', spell.id, spell.duration, 2 )
	target.partsys_id = game.particles( 'sp-Produce Flame', target.obj )

def OnBeginRound( spell ):
	print "Sunbeam OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Sunbeam OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Produce Flame-proj', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Searing Light', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Sunbeam OnEndProjectile"

	targg486 = spell.target_list[index_of_target].obj

	dam = dice_new( '1d6' )
	dam2 = dice_new( '4d6' )
	dam.number = min( 20, spell.caster_level )

	return_val = spell.caster.perform_touch_attack( targg486 )

	game.particles( 'sp-Searing Light-Hit', targg486 )

	if return_val & D20CAF_HIT:
		if targg486.is_category_type( mc_type_undead ) or targg486.is_category_type( mc_type_plant ) or targg486.is_category_type( mc_type_ooze ):

			if targg486.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_POSITIVE_ENERGY, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				targg486.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				targg486.float_mesfile_line( 'mes\\spell.mes', 20019 )
				targg486.condition_add_with_args( 'sp-Blindness', spell.id, 30000, 0 )

		else:

			if targg486.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam2, D20DT_FIRE, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ):
				# saving throw successful
				targg486.float_mesfile_line( 'mes\\spell.mes', 30001 )
			else:
				# saving throw unsuccessful
				targg486.float_mesfile_line( 'mes\\spell.mes', 20019 )
				targg486.condition_add_with_args( 'sp-Blindness', spell.id, 30000, 0 )


def OnEndSpellCast( spell ):
	print "Sunbeam OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Ray of Stupidity OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Ray of Stupidity OnSpellEffect"

def OnBeginRound( spell ):
	print "Ray of Stupidity OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Ray of Stupidity OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Searing Light', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Ray of Stupidity OnEndProjectile"

	target_item = spell.target_list[0]

	dice = dice_new( '1d4' )
	dice.bonus = 1
	dam_amount = dice.roll()	
	if dam_amount >= target_item.obj.stat_level_get(stat_intelligence):
		dam_amount = target_item.obj.stat_level_get(stat_intelligence) -1 
	dam_amount = -dam_amount 
	print "amount=", dam_amount

	spell.duration = 10 * spell.caster_level

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	if spell.caster.perform_touch_attack( target_item.obj ) & D20CAF_HIT:

		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 20022, tf_red )
		target_item.obj.condition_add_with_args( 'sp-Foxs Cunning', spell.id, spell.duration, dam_amount )
		target_item.partsys_id = game.particles( 'sp-Ray of Enfeeblement-Hit', target_item.obj )

	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnEndSpellCast( spell ):
	print "Ray of Stupidity OnEndSpellCast"
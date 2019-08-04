from toee import *
from utilities import  *
from Co8 import *

def OnBeginSpellCast( spell ):
	print "Avasculate OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	
	print "Avasculate OnSpellEffect"

def OnBeginRound( spell ):
	print "Avasculate OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Avasculate OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Disintegrate', projectile ) )


def OnEndProjectile( spell, projectile, index_of_target ):
	print "Avasculate OnEndProjectile"
	target_item = spell.target_list[0]
	
	spell.duration = 0

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

	return_val = spell.caster.perform_touch_attack( target_item.obj )
	if return_val & D20CAF_HIT:
		game.particles( 'sp-Curse Water', target_item.obj )
		game.particles( 'hit-BLUDGEONING_AND_PIERCING-medium', target_item.obj )
		game.particles( 'hit-BLUDGEONING-medium', target_item.obj )
		game.particles( 'hit-PIERCING_AND_SLASHING-medium', target_item.obj )
		game.particles( 'hit-SLASHING_AND_BLUDGEONING_AND_PIERCING-medium', target_item.obj )
		game.particles( 'hit-SLASHING-medium', target_item.obj )

		damage_dice = (target_item.obj.stat_level_get( stat_hp_current ) / 2)
		dam = dice_new( '1d1' )
		dam.number = min( 500, damage_dice )
		target_item.obj.spell_damage( spell.caster, D20DT_BLOOD_LOSS, dam, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ) # Avasculate doesn't really do damage so not using the weaponlike rule here
		if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		else:
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
			spell.duration = 1
			target_item.obj.condition_add_with_args( 'sp-Sound Burst', spell.id, spell.duration, 0 )
	else:

		# missed
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )

	if has_it == 1:
		spell.caster.stat_base_set(stat_dexterity, x)

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end(spell.id)

def OnEndSpellCast( spell ):
	print "Avasculate OnEndSpellCast"
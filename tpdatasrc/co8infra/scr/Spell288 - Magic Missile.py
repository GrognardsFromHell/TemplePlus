from toee import *

def OnBeginSpellCast( spell ):
	print "Magic Missile OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

	#spell.num_of_projectiles = spell.num_of_projectiles + 1
	#spell.target_list.push_target(spell.caster)     # didn't work :(
	# generally the sequence is: OnBeginSpellCast, OnBeginProjectile, OnSpellEffect,OnEndProjectile (OnBeginRound isn't called)

def OnSpellEffect( spell ):
	print "Magic Missile OnSpellEffect"


def OnBeginRound( spell ):
	print "Magic Missile OnBeginRound"


def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnBeginProjectile"

	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-magic missle-proj', projectile ) )
	

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Magic Missile OnEndProjectile"

	game.particles_end( projectile.obj_get_int( obj_f_projectile_part_sys_id ) )

	target = spell.target_list[ index_of_target ]

	damage_dice = dice_new( '1d4' )
	damage_dice.bonus = 1

	target_item_obj = target.obj
	if (not spell.caster in game.party[0].group_list() ) and target_item_obj.d20_query(Q_Critter_Is_Charmed ) :
		# NPC enemy is trying to cast on a charmed target - this is mostly meant for the Cult of the Siren encounter
		target_item_obj = party_closest( spell.caster, conscious_only= 1, mode_select= 1, exclude_warded= 1, exclude_charmed = 1) # select nearest conscious PC instead, who isn't already charmed
		if target_item_obj == OBJ_HANDLE_NULL:
			target_item_obj = target.obj

	# always hits
	target_item_obj.condition_add_with_args( 'sp-Magic Missile', spell.id, spell.duration, damage_dice.roll() )
	target.partsys_id = game.particles( 'sp-magic missle-hit', target_item_obj )

	# special scripting for NPCs no longer necessary - NPCs will launch multiple projectiles now

	#spell.target_list.remove_target_by_index( index_of_target )
	spell.num_of_projectiles -= 1

	if spell.num_of_projectiles == 0:
##		loc = target.obj.location
##		target.obj.destroy()
##		mxcr = game.obj_create( 12021, loc )
##		game.global_vars[30] = game.global_vars[30] + 1
		spell.spell_end( spell.id, 1 )

def OnEndSpellCast( spell ):
	print "Magic Missile OnEndSpellCast"
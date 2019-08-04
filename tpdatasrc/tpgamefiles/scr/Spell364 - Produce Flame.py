from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Produce Flame OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Produce Flame OnSpellEffect"

	xx,yy = location_to_axis(spell.caster.location)
	spell.duration = 10 * spell.caster_level
	if spell.caster_level > 5:
		spell.caster_level = 5


	target = spell.target_list[0]

	target.obj.condition_add_with_args( 'sp-Produce Flame', spell.id, spell.duration, 0 )
	target.partsys_id = game.particles( 'sp-Produce Flame', target.obj )

def OnBeginRound( spell ):
	print "Produce Flame OnBeginRound"

def OnBeginProjectile( spell, projectile, index_of_target ):
	print "Produce Flame OnBeginProjectile"

	#spell.proj_partsys_id = game.particles( 'sp-Produce Flame-proj', projectile )
	projectile.obj_set_int( obj_f_projectile_part_sys_id, game.particles( 'sp-Produce Flame-proj', projectile ) )

def OnEndProjectile( spell, projectile, index_of_target ):
	print "Produce Flame OnEndProjectile"
	spell.caster.d20_send_signal_ex( S_TouchAttack, spell.target_list[ index_of_target ].obj )

def OnEndSpellCast( spell ):
	print "Produce Flame OnEndSpellCast"
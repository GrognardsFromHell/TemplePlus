from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Produce Flame OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Produce Flame OnSpellEffect"

	xx,yy = location_to_axis(spell.caster.location)
	if game.leader.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
	# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
		game.particles( 'swirled gas', spell.caster )
		spell.caster.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
		game.sound(7581,1)
		game.sound(7581,1)
	else:
		spell.duration = 10* spell.caster_level
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

	targg364 = spell.target_list[index_of_target].obj
	xx,yy = location_to_axis(targg364.location)
	if targg364.map == 5067 and ( xx >= 521 and xx <= 555 ) and ( yy >= 560 and yy <= 610):
		# Water Temple Pool Enchantment prevents fire spells from working inside the chamber, according to the module -SA
		targg364.float_mesfile_line( 'mes\\skill_ui.mes', 2000 , 1 )
		game.sound(7581,1)
		game.sound(7581,1)
		game.particles( 'swirled gas', targg364 )
	else:
		#return_val = spell.caster.perform_touch_attack( targg364 )
		#if return_val >= 1:
			#damage_dice = dice_new( '4d6' )
			#game.particles( 'sp-Produce Flame-Hit', targg364 )
			#target.obj.spell_damage( spell.caster, D20DT_FIRE, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
		spell.caster.d20_send_signal_ex( S_TouchAttack, spell.target_list[ index_of_target ].obj )
		##else:
			# missed
			targg364.float_mesfile_line( 'mes\\spell.mes', 30007 )
			game.particles( 'Fizzle', targg364 )

def OnEndSpellCast( spell ):
	print "Produce Flame OnEndSpellCast"
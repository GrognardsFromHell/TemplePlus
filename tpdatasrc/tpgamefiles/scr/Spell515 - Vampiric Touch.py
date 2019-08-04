from toee import *

def OnBeginSpellCast( spell ):
	print "Vampiric Touch OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level 
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Vampiric Touch OnSpellEffect"

	dice = dice_new("1d6")
	dice.number = min(10, (spell.caster_level) / 2)
	spell.duration = 600
	target = spell.target_list[0]
	if not (target.obj == spell.caster):
		attack_successful = spell.caster.perform_touch_attack( target.obj , 1)

		if attack_successful & D20CAF_HIT:

			old_hp = target.obj.stat_level_get( stat_hp_current )
			target.obj.spell_damage_weaponlike( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, attack_successful, 0 )
			new_hp = target.obj.stat_level_get( stat_hp_current )
			damage = old_hp - new_hp
			if damage > (old_hp + 10):
				damage = old_hp + 10

			#spell.caster.condition_add_with_args( 'Temporary_Hit_Points', spell.id, spell.duration, damage )
			spell.caster.condition_add_with_args( 'sp-Vampiric Touch', spell.id, spell.duration, damage )
			spell.caster.float_mesfile_line( 'mes\\spell.mes', 20005, 0 )

		else:

			#target.obj.float_mesfile_line( 'mes\\spell.mes', 30021 )
			game.particles( 'Fizzle', target.obj )
			spell.target_list.remove_target( target.obj )
			game.particles( 'sp-Vampiric Touch', spell.caster )

def OnBeginRound( spell ):
	print "Vampiric Touch OnBeginRound"

def OnEndSpellCast( spell ):
	print "Vampiric Touch OnEndSpellCast"
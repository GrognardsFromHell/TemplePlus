from toee import *
from utilities import  * 

def OnBeginSpellCast( spell ):
	print "Healing Circle OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Healing Circle OnSpellEffect"

	remove_list = []

	dice = dice_new( '2d8' )
	#dice.bonus = min( 30, spell.caster.stat_level_get( spell.caster_class ) )
	dice.bonus = min( 30, spell.caster_level )

#	game.particles( 'sp-Healing Circle', spell.target_loc )

	for target_item in spell.target_list:

		target = target_item.obj

		# check if target is friendly (willing target)
		if target.is_friendly( spell.caster ):

			# check if target is undead
			if target.is_category_type( mc_type_undead ):
				# check saving throw, damage target
				if target.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
					target.float_mesfile_line( 'mes\\spell.mes', 30001 )

					# saving throw succesful, damage target, 1/2 damage
					target.spell_damage_with_reduction( spell.caster, D20DT_POSITIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
				else:
					target.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# saving throw unsuccesful, damage target, full damage
					target.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			else:
				# heal target
				target.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
				target.healsubdual( spell.caster, dice, D20A_CAST_SPELL, spell.id )

		else:

			# check if target is undead
			if target.is_category_type( mc_type_undead ):
				# check saving throw, damage target
				if target.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
					target.float_mesfile_line( 'mes\\spell.mes', 30001 )

					# saving throw succesful, damage target, 1/2 damage
					target.spell_damage_with_reduction( spell.caster, D20DT_POSITIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
				else:
					target.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# saving throw unsuccesful, damage target, full damage
					target.spell_damage( spell.caster, D20DT_POSITIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
			else:
				# check saving throw
				if target.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
					#target.float_mesfile_line( 'mes\\spell.mes', 30001 )

					# saving throw succesful, heal target, 1/2 heal
					target.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
					target.healsubdual( spell.caster, dice, D20A_CAST_SPELL, spell.id )
				else:
					#target.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# saving throw unsuccesful, heal target, full heal
					target.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
					target.healsubdual( spell.caster, dice, D20A_CAST_SPELL, spell.id )

		game.particles( 'sp-Cure Moderate Wounds', target )

		remove_list.append( target )
	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Healing Circle OnBeginRound"

def OnEndSpellCast( spell ):
	print "Healing Circle OnEndSpellCast"
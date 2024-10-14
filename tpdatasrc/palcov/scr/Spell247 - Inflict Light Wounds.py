from toee import *

def OnBeginSpellCast( spell ):
	print "Inflict Light Wounds OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Inflict Light Wounds OnSpellEffect"

	dice = dice_new( "1d8" )
	dice.bonus = min( 5, spell.caster_level )

	target = spell.target_list[0]

	# check if target is friendly (willing target)
	if target.obj.is_friendly( spell.caster ):

		# check if target is undead
		if target.obj.is_category_type( mc_type_undead ):
			target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
		else:
			# damage target
			if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				# saving throw succesful, damage target, 1/2 damage
				target.obj.spell_damage_with_reduction( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
			else:
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				# saving throw unsuccesful, damage target, full damage
				target.obj.spell_damage( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	else:

		# check if target is undead
		if target.obj.is_category_type( mc_type_undead ):
			# check saving throw, heal target
			if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				#target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				# saving throw succesful, heal target, 1/2 heal
				target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
			else:
				#target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				# saving throw unsuccesful, heal target, full heal
				target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
		else:
			# check saving throw, damage target
			if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				# saving throw succesful, damage target, 1/2 damage
#				target.obj.spell_damage( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
				target.obj.spell_damage_with_reduction( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
			else:
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				# saving throw unsuccesful, damage target, full damage
				target.obj.spell_damage( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	game.particles( 'sp-Inflict Light Wounds', target.obj )

	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Inflict Light Wounds OnBeginRound"

def OnEndSpellCast( spell ):
	print "Inflict Light Wounds OnEndSpellCast"
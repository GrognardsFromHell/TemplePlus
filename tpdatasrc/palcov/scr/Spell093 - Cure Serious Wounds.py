from toee import *

def OnBeginSpellCast( spell ):
	print "Cure Serious Wounds OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Cure Serious Wounds OnSpellEffect"

	npc = spell.caster			##  added so NPC's can use potion
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL and spell.caster_level <= 0:
		spell.caster_level = 10

	dice = dice_new( "3d8" )
	dice.bonus = min( 15, spell.caster_level )

	target = spell.target_list[0].obj

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

	game.particles( 'sp-Cure Serious Wounds', target )

	spell.target_list.remove_target( target )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Cure Serious Wounds OnBeginRound"

def OnEndSpellCast( spell ):
	print "Cure Serious Wounds OnEndSpellCast"
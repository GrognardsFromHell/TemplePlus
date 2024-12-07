from toee import *

def OnBeginSpellCast( spell ):
	print "Blindness/Deafness OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )
	return 1

def OnSpellEffect( spell ):
	print "Blindness/Deafness OnSpellEffect"

	if spell.caster.name == 14959:  # Nymph
		spell.dc = 17               # 10 + 3 + 4 (charisma)
		spell.caster_level = 7
		
	spell.duration = 14400
	target_item = spell.target_list[0]

	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2:
		spell_arg = 1  # blineness, marc

	# allow Fortitude saving throw to negate
	if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
		# saving throw successful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	else:
		# saving throw unsuccessful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		if spell_arg == 1:
			# apply blindness
			return_val = target_item.obj.condition_add_with_args( 'sp-Blindness', spell.id, spell.duration, 0 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Blindness-Deafness', target_item.obj )

		else:
			# apply deafness
			return_val = target_item.obj.condition_add_with_args( 'sp-Deafness', spell.id, spell.duration, 0 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Blindness-Deafness', target_item.obj )

	# Nymph
	if spell.caster.name == 14959:
		for pc in game.party:
			if pc != target_item.obj and spell.caster.distance_to(pc) <= 50:
				if pc.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
					pc.float_mesfile_line( 'mes\\spell.mes', 30001 )
					game.particles( 'Fizzle', pc )
				else:
					pc.float_mesfile_line( 'mes\\spell.mes', 30002 )
					return_val = pc.condition_add_with_args( 'sp-Blindness', spell.id, spell.duration, 0 )
					if return_val == 1:
						game.particles( 'sp-Blindness-Deafness', pc )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Blindness/Deafness OnBeginRound"

def OnEndSpellCast( spell ):
	print "Blindness/Deafness OnEndSpellCast"
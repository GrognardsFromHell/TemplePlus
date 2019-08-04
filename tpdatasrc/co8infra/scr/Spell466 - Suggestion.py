from toee import *

def OnBeginSpellCast( spell ):
	print "Suggestion OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Suggestion OnSpellEffect"

	npc = spell.caster
	if npc.name == 14358:	## Balor Guardian 	
		spell.dc = 27

	spell.duration = 600 * spell.caster_level
	target_item = spell.target_list[0]

	if not target_item.obj.is_friendly( spell.caster ):
		if (target_item.obj.type == obj_t_pc) or (target_item.obj.type == obj_t_npc):
			if not target_item.obj.is_category_type( mc_type_animal ):
				if target_item.obj.get_size < STAT_SIZE_HUGE:

					if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

						spell.caster.ai_follower_add( target_item.obj )

						target_item.obj.condition_add_with_args( 'sp-Suggestion', spell.id, spell.duration, 0 )
						target_item.partsys_id = game.particles( 'sp-Suggestion', target_item.obj )

						# add target to initiative, just in case
						target_item.obj.add_to_initiative()
						game.update_combat_ui()

					else:

						# saving throw successful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

						game.particles( 'Fizzle', target_item.obj )
						spell.target_list.remove_target( target_item.obj )

				else:
					# not medium sized or smaller
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31005 )

					game.particles( 'Fizzle', target_item.obj )
					spell.target_list.remove_target( target_item.obj )

			else:
				# a monster
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )

		else:
			# not a person
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31001 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	else:

		# can't target friendlies
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Suggestion OnBeginRound"

def OnEndSpellCast( spell ):
	print "Suggestion OnEndSpellCast"
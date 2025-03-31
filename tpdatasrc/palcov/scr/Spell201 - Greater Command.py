from toee import *

def OnBeginSpellCast( spell ):
	print "Greater Command OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Greater Command OnSpellEffect"

	remove_list = []

	spell.duration = 1 * spell.caster_level

	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2 and spell_arg != 3 and spell_arg != 4:
		spell_arg = 2
	npc = spell.caster
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		spell_arg = 2

	for target_item in spell.target_list:
		if not target_item.obj.is_friendly( spell.caster ):
			if (target_item.obj.type == obj_t_pc) or (target_item.obj.type == obj_t_npc):
				if not target_item.obj.is_category_type( mc_type_animal ):
					if target_item.obj.get_size < STAT_SIZE_LARGE:

						if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
							# saving throw unsuccessful
							target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

							target_item.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, spell_arg )
							target_item.partsys_id = game.particles( 'sp-Command', target_item.obj )

							# add target to initiative, just in case
							#target_item.obj.add_to_initiative()
							#game.update_combat_ui()

						else:

							# saving throw successful
							target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

							game.particles( 'Fizzle', target_item.obj )
							remove_list.append( target_item.obj )

					else:
						# not medium sized or smaller
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31005 )

						game.particles( 'Fizzle', target_item.obj )
						remove_list.append( target_item.obj )

				else:
					# a monster
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

					game.particles( 'Fizzle', target_item.obj )
					remove_list.append( target_item.obj )

			else:
				# not a person
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31001 )

				game.particles( 'Fizzle', target_item.obj )
				remove_list.append( target_item.obj )

		else:

			# can't target friendlies
			game.particles( 'Fizzle', target_item.obj )
			remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Greater Command OnBeginRound"

def OnEndSpellCast( spell ):
	print "Greater Command OnEndSpellCast"
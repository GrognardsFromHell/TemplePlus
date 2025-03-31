from utilities import *
from toee import *

import _include
from co8Util.PersistentData import *
from co8Util.ObjHandling import *
from co8Util.spells import *

def OnBeginSpellCast( spell ):
	print "Command OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Command OnSpellEffect"

	spell.duration = 1
	target_item = spell.target_list[0]

	npc = spell.caster			##  added so NPC's will choose valid targets
	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2 and spell_arg != 3 and spell_arg != 4:
		spell_arg = 2
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		spell_arg = 2

	if not target_item.obj.is_friendly( spell.caster ):
		if (target_item.obj.type == obj_t_pc) or (target_item.obj.type == obj_t_npc):
			if not target_item.obj.is_category_type( mc_type_animal ):
				#if ( target_item.obj.get_size < STAT_SIZE_LARGE or target_item.obj.is_category_type(mc_type_humanoid) ):
				if 1:  # let creatures of STAT_SIZE_LARGE be affected too, marc

					if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id):
						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
						if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
							if target_item.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 2 ):
								add_to_persistent_list (COMMAND_KEY, spell.id, target_item.obj)  # marc
						else:			##  added so NPC's can cast Command
							if target_item.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, spell_arg ):
								add_to_persistent_list (COMMAND_KEY, spell.id, target_item.obj)  # marc
						target_item.partsys_id = game.particles( 'sp-Command', target_item.obj )

						# add target to initiative, just in case
						#target_item.obj.add_to_initiative()
						#game.update_combat_ui()

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
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30003 )
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Command OnBeginRound"

def OnEndSpellCast( spell ):
	print "Command OnEndSpellCast"
	remove_from_persistent_list (COMMAND_KEY, spell.id)  # marc

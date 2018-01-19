from utilities import *
from toee import *

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



##	if npc.name == 14424 or npc.name = 8091:			##  added so NPC's will choose valid targets
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:

		if not target_item.obj.is_category_type( mc_type_animal ) and (target_item.obj.get_size < STAT_SIZE_LARGE or target_item.obj.is_category_type(mc_type_humanoid) ) and critter_is_unconscious(target_item.obj) != 1 and not target_item.obj.d20_query(Q_Prone):
			npc = spell.caster

		else:
			#ffws += 'Target NOT Okayed!\n'

			game.global_flags[811] = 0	
			for obj in game.party[0].group_list():

				if obj.distance_to(npc) <= 5 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 10 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 15 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 20 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 25 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 30 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 100 and critter_is_unconscious(obj) != 1 and not obj.is_category_type( mc_type_animal ) and obj.get_size < STAT_SIZE_LARGE and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone):
					target_item.obj = obj
					game.global_flags[811] = 1


	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2 and spell_arg != 3 and spell_arg != 4:
		spell_arg = 2
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		spell_arg = 2


	if not target_item.obj.is_friendly( spell.caster ):
		if (target_item.obj.type == obj_t_pc) or (target_item.obj.type == obj_t_npc):
			if not target_item.obj.is_category_type( mc_type_animal ):
				if ( target_item.obj.get_size < STAT_SIZE_LARGE or target_item.obj.is_category_type(mc_type_humanoid) ):

					if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
						if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
							target_item.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 2 )
						else:			##  added so NPC's can cast Command
							target_item.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, spell_arg )
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
					# Note: I've added an exception for humanoids (i.e. magically enlarged party members) -SA
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
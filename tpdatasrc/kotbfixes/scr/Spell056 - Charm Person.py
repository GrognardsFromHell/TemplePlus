from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Charm Person OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Charm Person OnSpellEffect"

	spell.duration = 600 * spell.caster_level
	target_item = spell.target_list[0]
	target_item_obj = target_item.obj
	
	if game.global_vars[451] & 2**2 != 0:
		if game.combat_is_active():
			spell.dc = spell.dc - 5 # to reflect a bonus to the saving throw for casting charm in combat
	
	if (not spell.caster in game.party) and (target_item.obj.type != obj_t_pc) and (target_item.obj in game.party):
		# NPC enemy is trying to charm an NPC from your party - this is bad because it effectively kills the NPC (is dismissed from party and becomes hostile, thus becoming unrecruitable unless you use dominate person/monster)
		target_item_obj = party_closest( spell.caster, conscious_only= 1, mode_select= 0, exclude_warded= 0, exclude_charmed = 1) # select nearest conscious PC instead, who isn't already charmed
		if target_item_obj == OBJ_HANDLE_NULL:
			target_item_obj = target_item.obj

	if not target_item_obj.is_friendly( spell.caster ):
		if (target_item_obj.is_category_type( mc_type_humanoid )) and (target_item_obj.get_size < STAT_SIZE_LARGE):

			if not target_item_obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				# saving throw unsuccessful
				
				if target_item_obj.name == 14072 and target_item_obj.map == 5001: # Billy
					game.global_vars[122] |= 2**0 # Billy is charmed
						#if game.global_vars[7] == 0:
							#game.global_vars[7] = 2 # what if the guards are dead?
						#game.global_vars[122] |= 2**0 # Billy Charmed
					target_item_obj.scripts[9] = 439 # Billy's dlg
					for pc in game.party:
						target_item_obj.ai_shitlist_remove( pc )
					for pc in game.party:
						if is_safe_to_talk(target_item_obj, pc):
							game.timevent_add(pc.begin_dialog,( target_item_obj, 200 ), 1000 )
							break
				target_item_obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				spell.caster.ai_follower_add( target_item_obj )

				target_item_obj.condition_add_with_args( 'sp-Charm Person', spell.id, spell.duration, target_item.obj.hit_dice_num )
				target_item.partsys_id = game.particles( 'sp-Charm Person', target_item_obj )

				# add target to initiative, just in case
				target_item_obj.add_to_initiative()
				game.update_combat_ui()

			else:

				# saving throw successful
				target_item_obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item_obj )
				spell.target_list.remove_target( target_item_obj )

		else:
			# not a humanoid
			target_item_obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item_obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item_obj )

	else:

		# can't target friendlies
		game.particles( 'Fizzle', target_item_obj )
		spell.target_list.remove_target( target_item_obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Charm Person OnBeginRound"

def OnEndSpellCast( spell ):
	print "Charm Person OnEndSpellCast"

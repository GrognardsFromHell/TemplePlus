from toee import *
from utilities import *
from exclusions import *

def OnBeginSpellCast( spell ):
	print "Charm Person OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Charm Person OnSpellEffect"

	if spell.caster.name in [14937]:    # Nixie
		spell.dc = 15                   # 10 + 1 + 4 (charisma)
		spell.caster_level = 4
	elif spell.caster.name in [14507]:  # Dryad
		spell.dc = 13                   # 10 + 1 + 2 (wisdom)
		spell.caster_level = 6

	spell.duration = 600 * spell.caster_level
	target_item = spell.target_list[0]
	target_item_obj = target_item.obj

	if game.combat_is_active():
		spell.dc = spell.dc - 5 # to reflect a bonus to the saving throw for casting charm in combat

	if not target_item_obj.is_friendly( spell.caster ) and target_item_obj.name not in charm_exclusions:
		if (target_item_obj.is_category_type( mc_type_humanoid )) and (target_item_obj.get_size < STAT_SIZE_LARGE):

			if not target_item_obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id, spell.id ):
				# saving throw unsuccessful
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
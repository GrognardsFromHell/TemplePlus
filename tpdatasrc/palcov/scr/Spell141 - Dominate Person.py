from toee import *
from exclusions import *

def OnBeginSpellCast( spell ):
	print "Dominate Person OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Dominate Person OnSpellEffect"

	if spell.caster.name in [14286,14358]:	# Balor 	
		spell.dc = 27			# 10 + 9 (Dominate Monster) + 8 (Cha) 

	spell.duration = 14400 * spell.caster_level
	target_item = spell.target_list[0]

	if not target_item.obj.is_friendly( spell.caster ) and target_item.obj.name not in charm_exclusions:

		if (target_item.obj.is_category_type( mc_type_humanoid ) == 1):

			if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id):
				# saving throw unsuccessful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				target_item.obj.condition_add_with_args( 'sp-Dominate Person', spell.id, spell.duration, target_item.obj.hit_dice_num )
				target_item.partsys_id = game.particles( 'sp-Dominate Person', target_item.obj )

				# add target to initiative, just in case
				target_item.obj.add_to_initiative()
				game.update_combat_ui()

			else:

				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				game.particles( 'Fizzle', target_item.obj )
				spell.target_list.remove_target( target_item.obj )

		else:

			# not an humanoid
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30000 )
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31004 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )

	else:

		# can't target friendlies
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Dominate Person OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dominate Person OnEndSpellCast"
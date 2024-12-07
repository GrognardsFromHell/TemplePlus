from toee import *

def OnBeginSpellCast( spell ):
	print "Banishment OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Banishment OnSpellEffect"

	remove_list = []

	spell.duration = 0
	hitDiceAmount = 2 * spell.caster_level
	banish_casterLV = spell.caster_level

	# check for any item that is distasteful to the subjects (Needs suggestions)
	bonus1_list = [8028]		## Potion of Protection From Outsiders
	for bonus1 in bonus1_list:
		if spell.caster.item_find(bonus1) != OBJ_HANDLE_NULL:
			spell.dc = spell.dc + 2				## the saving throw DC increases by 2
			# does NOT work! (Needs a fix.)
			# spell.caster_level = spell.caster_level + 1	## +1 bonus on your caster level check for overcoming Spell Resistance

	# check for rare items that work twice as well as a normal item for the purpose of the bonuses (Needs suggestions)
	bonus2_list = [12900]		## Swamp Lotus
	for bonus2 in bonus2_list:
		if spell.caster.item_find(bonus2) != OBJ_HANDLE_NULL:
			spell.dc = spell.dc + 4				## the saving throw DC increases by 4
			# does NOT work! (Needs a fix.)
			# spell.caster_level = spell.caster_level + 2	## +2 bonus on your caster level check for overcoming Spell Resistance

	for target_item in spell.target_list:

		# check critter hit dice
		targetHitDice = target_item.obj.hit_dice_num

		# check if target does not exceed the amount allowed
		if hitDiceAmount >= targetHitDice:

			# spell.dc is DC - target's HD + caster's caster_level
			#- spell.dc = spell.dc - targetHitDice + banish_casterLV
			#- spell.dc = max( 1, spell.dc )

			# Fix for DC growing out of control, marc.
			special_save_dc = spell.dc - targetHitDice + banish_casterLV
			special_save_dc = max( 1, special_save_dc )

			if (target_item.obj.type == obj_t_npc):

				# check target is EXTRAPLANAR
				if target_item.obj.d20_query_has_spell_condition( sp_Summoned ) or target_item.obj.is_category_type( mc_type_outsider ) or target_item.obj.is_category_subtype(mc_subtype_extraplanar):

					# subtract the target's hit dice from the amount allowed
					hitDiceAmount = hitDiceAmount - targetHitDice

					# allow Will saving throw to negate
					if target_item.obj.saving_throw_spell( special_save_dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
						# saving throw successful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
						game.particles( 'Fizzle', target_item.obj )
	
					else:
						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
	
						# creature is sent back to its own plane (no 20% of wrong plane, DUMB)
						# kill for now
						# So you'll get awarded XP for the kill
						if not target_item.obj in game.leader.group_list():
							target_item.obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
						target_item.obj.critter_kill()
						if target_item.obj.d20_query_has_spell_condition( sp_Summoned ):
							game.particles( 'sp-Dismissal', target_item.obj.location )
							target_item.obj.destroy()
						else:
							game.particles( 'sp-Dismissal', target_item.obj )
							target_item.obj.condition_add_with_args( 'sp-Animate Dead', spell.id, spell.duration, 3 )

				else:
					# target is not EXTRAPLANAR
					target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31007 )
					game.particles( 'Fizzle', target_item.obj )

			else:
				# target is not an NPC
				game.particles( 'Fizzle', target_item.obj )

		else:
			# ran out of allowed HD
			game.particles( 'Fizzle', target_item.obj )

		remove_list.append( target_item.obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Banishment OnBeginRound"

def OnEndSpellCast( spell ):
	print "Banishment OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Sleep OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

	# HTN - sort the list by hitdice
	spell.spell_target_list_sort( SORT_TARGET_LIST_BY_HIT_DICE_THEN_DIST, SORT_TARGET_LIST_ORDER_ASCENDING )
	print "target_list sorted by hitdice and dist from target_Loc (least to greatest): ", spell.target_list

def OnSpellEffect( spell ):
	print "Sleep OnSpellEffect"

	remove_list = []

	spell.duration = 10 * spell.caster_level
	hit_dice_max = 4
	#hit_dice_roll = dice_new( '2d4' )
	#hit_dice_max = hit_dice_roll.roll()

	#print "sleep, can affect a total of (", hit_dice_max, ") HD"

	game.particles( 'sp-Sleep', spell.target_loc )

	# get all targets in a 15ft radius
	for target_item in spell.target_list:

		# check critter_hit_dice
		obj_hit_dice = target_item.obj.hit_dice_num

		if (obj_hit_dice < 5) and (hit_dice_max >= obj_hit_dice):

			# subtract the obj.hit_dice from the max
			hit_dice_max = hit_dice_max - obj_hit_dice

			# allow Will saving throw to negate
			if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
	
				# saving throw successful
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
	
				game.particles( 'Fizzle', target_item.obj )
				remove_list.append( target_item.obj )
	
			else:
	
				# saving throw unsuccessful
				# if game.leader.map == 5005 and target_item.obj.name in range(14074, 14078):
					# a123 = target_item.obj.item_worn_at(3).obj_get_int(obj_f_weapon_type) 
					# if a123 in [14, 17, 46,  48, 68]: # (Is archer)
						# snorer = target_item.obj
						# pad3 = snorer.obj_get_int(obj_f_npc_pad_i_3)
						# pad3 |= 2**9
						# snorer.obj_set_int(obj_f_npc_pad_i_3, pad3)
						# snorer.obj_set_int(obj_f_pad_i_0, snorer.obj_get_int(obj_f_critter_strategy) ) # Record original strategy
						# snorer.obj_set_int(obj_f_critter_strategy, 88) # "Archer stay put" strat
				
				target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
				target_item.obj.condition_add_with_args( 'sp-Sleep', spell.id, spell.duration, 0 )
			
				#################################################################
				# Added by Sitra Achara - Moathouse "Wakey Wakey" scripting	#
				# obj_f_npc_pad_i_3 - bit "7" (2**7)				#
				#################################################################
				
				# if game.leader.map == 5005 and target_item.obj.name in range(14074, 14078):
					# snorer = target_item.obj
					# pad3 = snorer.obj_get_int(obj_f_npc_pad_i_3)
					# pad3 |= 2**7
					# snorer.obj_set_int(obj_f_npc_pad_i_3, pad3)
					
					# no longer necessary with TemplePLus


		else:

			game.particles( 'Fizzle', target_item.obj )
			remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id) # changed back force_spell_end to 0 despite there being no per-round effects. it caused problems - sp-Sleep would get pruned when saving...

def OnBeginRound( spell ):
	print "Sleep OnBeginRound"

def OnEndSpellCast( spell ):
	print "Sleep OnEndSpellCast"
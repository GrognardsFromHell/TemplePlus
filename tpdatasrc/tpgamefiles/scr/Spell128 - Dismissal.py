from toee import *

def OnBeginSpellCast( spell ):
	print "Dismissal OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Dismissal OnSpellEffect"

	target_item = spell.target_list[0]

	spell.duration = 0

	# spell.dc is DC - target's HD + caster's caster_level
	spell.dc = spell.dc - target_item.obj.hit_dice_num + spell.caster_level
	spell.dc = max( 1, spell.dc )

	if (target_item.obj.type == obj_t_npc):

		if target_item.obj.d20_query_has_spell_condition( sp_Summoned ) or target_item.obj.is_category_type( mc_type_outsider ) or target_item.obj.is_category_subtype(mc_subtype_extraplanar):


			# allow Will saving throw to negate
			if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
	
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
				game.particles( 'sp-Dismissal', target_item.obj )

		else:

			# target is not EXTRAPLANAR
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31007 )

			game.particles( 'Fizzle', target_item.obj )

	else:

		# target is not an NPC
		game.particles( 'Fizzle', target_item.obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Dismissal OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dismissal OnEndSpellCast"
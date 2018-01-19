from toee import *

def OnBeginSpellCast( spell ):
	print "Blasphemy OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Blasphemy OnSpellEffect"

	remove_list = []

	# The Will save versus banishment is at a -4 penalty
	spell.dc = spell.dc + 4

	npc = spell.caster
	npc_caster_level = spell.caster_level
	if npc.name == 14286 or npc.name == 14358: # Balors
		npc_caster_level = 20
		spell.dc = 25+4 #only affects banishment anyway


	for target_item in spell.target_list:
		obj_hit_dice = target_item.obj.hit_dice_num

		# Only works on non-evil creatures
		alignment = target_item.obj.critter_get_alignment()
		if not (alignment & ALIGNMENT_EVIL) and not (npc == target_item.obj):
			game.particles( 'sp-Slay Living', target_item.obj )

			# Anything ten or more levels below the caster's level dies
			if obj_hit_dice <= (npc_caster_level - 10):
				# So you'll get awarded XP for the kill
				if not target_item.obj in game.leader.group_list():
					target_item.obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
				target_item.obj.critter_kill()

			# Anything five or more levels below the caster's level is paralyzed
			if obj_hit_dice <= (npc_caster_level - 5):
				spell.duration = game.random_range(1,10) * 10
				target_item.obj.condition_add_with_args( 'sp-Hold Monster', spell.id, spell.duration, 0 )

			# Anything one or more levels below the caster's level is weakened
			if obj_hit_dice <= (npc_caster_level - 1): 
				spell.duration = game.random_range(1,4) + game.random_range(1,4)
				dam_amount = game.random_range(1,6) + game.random_range(1,6)
				target_item.obj.condition_add_with_args( 'sp-Ray of Enfeeblement', spell.id, spell.duration, dam_amount )

			# Anything the caster's level or below is dazed
			if obj_hit_dice <= (npc_caster_level):
				spell.duration = 1
				target_item.obj.condition_add_with_args( 'sp-Daze', spell.id, spell.duration, 0 )

				# Summoned and extraplanar creatures below the caster's level are also banished
				# if they fail a Will save at -4
				if target_item.obj.d20_query_has_spell_condition( sp_Summoned ) or target_item.obj.npc_flags_get() & ONF_EXTRAPLANAR != 0:

					# allow Will saving throw to negate
					if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
	
						# saving throw successful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
						game.particles( 'Fizzle', target_item.obj )

					else:

						# saving throw unsuccessful
						target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
	
						# creature is sent back to its own plane
						# kill for now
						target_item.obj.critter_kill()

		remove_list.append( target_item.obj )
	#f.close()

	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

	

def OnBeginRound( spell ):
	print "Blasphemy OnBeginRound"

def OnEndSpellCast( spell ):
	print "Blasphemy OnEndSpellCast"
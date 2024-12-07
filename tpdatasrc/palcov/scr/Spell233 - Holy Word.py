from toee import *

def OnBeginSpellCast( spell ):
	print "Holy Word OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Holy Word OnSpellEffect"

	remove_list = []

	# The Will save versus banishment is at a -4 penalty
	spell.dc = spell.dc + 4
	
	
	obj_list = []
	# extracting the obj's so we don't get changed iterator bullshit (see http://www.co8.org/community/index.php?threads/holy-word-killed-my-cg-pc.12164/#post-145537)
	for target_item in spell.target_list:
		# Only works on non-good creatures
		target_item_obj = target_item.obj
		alignment = target_item_obj.critter_get_alignment()
		
		if  not (alignment & ALIGNMENT_GOOD):
			obj_list.append(target_item_obj )
		remove_list.append( target_item_obj )
	
	#for target_item in spell.target_list:
		#target_item_obj = target_item.obj
	for target_item_obj in obj_list:
	
		print "target item: " + str(target_item_obj)
		obj_hit_dice = target_item_obj.hit_dice_num
						
		game.particles( 'sp-Holy Smite', target_item_obj )

		# Anything ten or more levels below the caster's level dies
		if obj_hit_dice <= (spell.caster_level - 10):
			if not target_item_obj in game.leader.group_list():
				#print "Inflicting damage due to low critter HD: " + str(target_item_obj)
				target_item_obj.damage( game.leader , D20DT_POSITIVE_ENERGY, dice_new("52d52"))
				# So you'll get awarded XP for the kill
			#print "Killing due to low critter HD: " + str(target_item_obj)
			target_item_obj.critter_kill()

		# Anything five or more levels below the caster's level is paralyzed
		if obj_hit_dice <= (spell.caster_level - 5):
			spell.duration = game.random_range(1,10) * 10
			target_item_obj.condition_add_with_args( 'sp-Hold Monster', spell.id, spell.duration, 0 )

		# Anything one or more levels below the caster's level is blinded
		if obj_hit_dice <= (spell.caster_level - 1): 
			spell.duration = game.random_range(1,4) + game.random_range(1,4)
			target_item_obj.condition_add_with_args( 'sp-Blindness', spell.id, spell.duration, 0 )

		# Anything the caster's level or below is deafened
		if obj_hit_dice <= (spell.caster_level):
			spell.duration = game.random_range(1,4)
			target_item_obj.condition_add_with_args( 'sp-Deafness', spell.id, spell.duration, 0 )

			# Summoned and extraplanar creatures below the caster's level are also banished
			# if they fail a Will save at -4
			if target_item_obj.d20_query_has_spell_condition( sp_Summoned ) or target_item_obj.npc_flags_get() & ONF_EXTRAPLANAR != 0:

				# allow Will saving throw to negate
				if target_item_obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

					# saving throw successful
					target_item_obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
					game.particles( 'Fizzle', target_item_obj )

				else:
					# saving throw unsuccessful
					target_item_obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# creature is sent back to its own plane
					# kill for now
					if not target_item_obj in game.leader.group_list():
						#print "Inflicting damage due to Summoned/Extraplanar: " + str(target_item_obj)
						target_item_obj.damage( game.leader , D20DT_POSITIVE_ENERGY, dice_new("52d52"))
						# So you'll get awarded XP for the kill
						
					#print "critter_kill due to Summoned or Extraplanar: " + str(target_item_obj)
					target_item_obj.critter_kill()
	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Holy Word OnBeginRound"

def OnEndSpellCast( spell ):
	print "Holy Word OnEndSpellCast"
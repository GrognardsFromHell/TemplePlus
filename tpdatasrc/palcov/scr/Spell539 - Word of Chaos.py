from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Word of Chaos OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect ( spell ):
	print "Word of Chaos OnSpellEffect"

	remove_list = []

	# The Will save versus banishment is at a -4 penalty
	spell.dc = spell.dc + 4

	npc = spell.caster
	npc_caster_level = spell.caster_level
	if npc.name == 14286 or npc.name == 14358: # Balors
		npc_caster_level = 20
		spell.dc = 25+4 #only affects banishment anyway

	#f = open('wordofchaos_feedback.txt', 'w')
	#f.write( 'ccc\n' )
	target_list = pyspell_targetarray_copy_to_obj_list(spell)

	for target_item_obj in target_list:
		#f.write( str(target_item_obj.name) + '\n' )
		obj_hit_dice = target_item_obj.hit_dice_num
		is_confused = 0

		# Only works on non-chaotic creatures
		alignment = target_item_obj.critter_get_alignment()
		if not (alignment & ALIGNMENT_CHAOTIC) and not (npc == target_item_obj):
			game.particles( 'sp-Polymorph Other', target_item_obj )

			# Anything ten or more levels below the caster's level dies
			if obj_hit_dice <= (npc_caster_level - 10):
				# So you'll get awarded XP for the kill
				if not target_item_obj in game.leader.group_list():
					target_item_obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
				target_item_obj.critter_kill()

			# Anything five or more levels below the caster's level is confused
			if obj_hit_dice <= (npc_caster_level - 5):
				spell.duration = game.random_range(1,10) * 10
				target_item_obj.float_mesfile_line( 'mes\\combat.mes', 113, tf_red )
				target_item_obj.condition_add_with_args( 'sp-Confusion', spell.id, spell.duration, 0 )
				is_confused = 1			## added because Confusion will end when either the stun or deafness spell end (Confusion needs a fix.)

			# Anything one or more levels below the caster's level is stunned
			if obj_hit_dice <= (npc_caster_level - 1):
				if is_confused == 0:		## added because Confusion will end when the stun spell ends
					spell.duration = 0
					target_item_obj.condition_add_with_args( 'sp-Color Spray Stun', spell.id, spell.duration, 0 )

			# Anything the caster's level or below is deafened
			if obj_hit_dice <= (npc_caster_level):
				if is_confused == 0:		## added because Confusion will end when the deafness spell ends
					spell.duration = game.random_range(1,4)
					target_item_obj.condition_add_with_args( 'sp-Shout', spell.id, spell.duration, 0 )

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
						# So you'll get awarded XP for the kill
						if not target_item_obj in game.leader.group_list():
							target_item_obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
						target_item_obj.critter_kill()

		remove_list.append( target_item_obj )
	#f.close()

	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Word of Chaos OnBeginRound"

def OnEndSpellCast( spell ):
	print "Word of Chaos OnEndSpellCast"
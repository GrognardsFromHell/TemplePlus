from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Dispel Magic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-abjuration-conjure", spell.caster )

def OnSpellEffect ( spell ):

	# Max caster level for Dispel Magic is 10, marc
	spell.caster_level = min(10, spell.caster_level)

	if spell.caster.name == 14938:    # Pixie
		spell.caster_level = 8
	elif spell.caster.name == 14950:  # Noble Salamander
		spell.caster_level = 15
	elif spell.caster.name == 14155:  # Alrem Ghost (works as 7th level spell)
		spell.caster_level = 15
	elif spell.caster.name == 14775:  # Concubines, just to make them more effective
		spell.caster_level = 10
	elif spell.caster.name == 14985:  # Nightwalker
		spell.caster_level = 21
	
	# Make a normal list, as spell.target_list only attempts to dispel half the targets, marc
	spell.spell_target_list_sort (SORT_TARGET_LIST_BY_DIST, SORT_TARGET_LIST_ORDER_ASCENDING)
	targets = []
	for t in spell.target_list:
		targets.append(t.obj)

	# NPC will use break enchantment, since condition "sp-Dispel Magic" will not target spells of 4th level or higher, marc
	if spell.caster not in game.party:
		float_mes(spell.caster,352,2)
		target = targets[0]
		if (target.type == obj_t_pc) or (target.type == obj_t_npc):
			game.particles( 'sp-Dispel Magic - Targeted', target )
			target.condition_add_with_args( 'sp-Break Enchantment', spell.id, spell.duration, 0 )

	# check if we are targeting an object or an area
	elif spell.is_object_selected() == 1:
		float_mes(spell.caster,352,2)
	
		#- target = spell.target_list[0]
		target = targets[0]

		# support dispel on critters
		#-if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):
		#-	target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
		if (target.type == obj_t_pc) or (target.type == obj_t_npc):
			game.particles( 'sp-Dispel Magic - Targeted', target )
			target.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 0 )

		# support dispel on portals and containers
		elif (target.obj.type == obj_t_portal) or (target.obj.type == obj_t_container):
			if target.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
				target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
				target.obj.portal_flag_unset( OPF_MAGICALLY_HELD )
				spell.target_list.remove_target( target.obj )

		# support dispel on these obj_types: weapon, ammo, armor, scroll
		# NO support for: money, food, key, written, generic, scenery, trap, bag
		#elif (target.obj.type == obj_t_weapon) or (target.obj.type == obj_t_ammo) or (target.obj.type == obj_t_armor) or (target.obj.type == obj_t_scroll):
			#print "[dispel magic] - items not supported yet!"
			#game.particles( 'Fizzle', target.obj )
			#spell.target_list.remove_target( target.obj )

	else:
		float_mes(spell.caster,353,2)

		# draw area effect particles
		game.particles( 'sp-Dispel Magic - Area', spell.target_loc )

		#- for target in spell.target_list:
		for target in targets:
			float_num(target, max( int(target.distance_to(spell.caster)), 0 ), 2)

			#- if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):
			if (target.type == obj_t_pc) or (target.type == obj_t_npc):

				#- target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
				game.particles( 'sp-Dispel Magic - Targeted', target)

				# fourth argument:  0 = dispel all,  1 = dispel once
				# target.obj is removed from spell.target_list during the call, marc

				#- target.obj.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 1 )
				target.condition_add_with_args( 'sp-Dispel Magic', spell.id, 0, 1 )

			# Commented out this code, which will never do anything, marc
			# Doors do not get targeted in the area of the spell.
			# Containers do,  but will not have portal flags set.

			# support dispel on portals and containers
			#- elif (target.obj.type == obj_t_portal) or (target.obj.type == obj_t_container):
			#-	if target.obj.portal_flags_get() & OPF_MAGICALLY_HELD:
			#-		target.partsys_id = game.particles( 'sp-Dispel Magic - Targeted', target.obj )
			#-		target.obj.portal_flag_unset( OPF_MAGICALLY_HELD )
			#-		spell.target_list.remove_target( target.obj )

			# support dispel on these obj_types: weapon, ammo, armor, scroll
			# NO support for: money, food, key, written, generic, scenery, trap, bag
			#elif (target.obj.type == obj_t_weapon) or (target.obj.type == obj_t_ammo) or (target.obj.type == obj_t_armor) or (target.obj.type == obj_t_scroll):
				#print "[dispel magic] - items not supported yet!"
				#game.particles( 'Fizzle', target.obj )
				#spell.target_list.remove_target( target.obj )

	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Dispel Magic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Dispel Magic OnEndSpellCast"
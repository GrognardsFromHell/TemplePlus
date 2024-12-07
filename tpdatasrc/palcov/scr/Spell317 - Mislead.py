from toee import *
from casters import staff_has, staff_stats

mislead_spell_list = {}


def OnBeginSpellCast( spell ):
	print "Mislead OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-illusion-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Mislead OnSpellEffect"

	if staff_has(spell.caster) == 1:
		CL, mod = staff_stats (spell.caster, '10011')
		spell.caster_level = CL
		
	spell.duration = 1 * spell.caster_level
	target_item = spell.target_list[0]

	num_of_images = spell.caster_level

	#print "num of images=", num_of_images

	game.particles( 'sp-Mirror Image', target_item.obj )
	target_item.obj.condition_add_with_args( 'sp-Mirror Image', spell.id, spell.duration, num_of_images )
	#target_item.partsys_id = game.particles( 'sp-Mirror Image', target_item.obj )

#	spell.id = spell.id + 1

	target_item.obj.condition_add_with_args( 'sp-Improved Invisibility', spell.id, spell.duration, 0 )
	target_item.partsys_id = game.particles( 'sp-Improved Invisibility', target_item.obj )

	# spell.spell_end( spell.id, 1 )# do not end the spell, else the effect countdown is interrupted

def OnBeginRound( spell ):
	# Crappy workaround to end the spell (otherwise it never ends...)
	# Note: OnBeginRound gets called num_of_images times each round, because sp-Mirror Image duplicates the target num_of_images times (to store the particle FX)
	# Thus we check the game time to prevent decrementing the duration multiple times
	cur_time = game.time.time_game_in_seconds(game.time)
	if spell.id in mislead_spell_list:
		entry = mislead_spell_list[spell.id]
		entry_time = entry[1]
		if cur_time > entry_time:
			entry[1] = cur_time
			entry[0] -= 1
			#mislead_spell_list[spell.id] = entry
			print "Mislead OnBeginRound, duration: " + str(entry[0]) + ", ID: " + str(spell.id)
			if entry[0] <= 0:
				spell.spell_end(spell.id, 1)
	else:
		print "Mislead OnBeginRound, duration: " + str(spell.duration) + ", ID: " + str(spell.id)
		mislead_spell_list[spell.id] = [spell.duration - 1, cur_time ]

def OnEndSpellCast( spell ):
	print "Mislead OnEndSpellCast"
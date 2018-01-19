from toee import *

from Co8 import *

def OnBeginSpellCast( spell ):
	print "Reincarnation OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Reincarnation OnSpellEffect"

	spell.duration = 1
	target_item = spell.target_list[0]

	target_item.obj.condition_add_with_args( 'sp-Raise Dead', spell.id, spell.duration, 0 )
	#target_item.partsys_id = game.particles( 'sp-Raise Dead', target_item.obj )

#	target_item.obj.stat_base_set(stat_race, (race_gnome))

	if target_item.obj.stat_level_get( stat_level ) == 1:
		game.global_vars[752] = 0
	elif target_item.obj.stat_level_get( stat_level ) == 2:
		game.global_vars[752] = 500
	elif target_item.obj.stat_level_get( stat_level ) == 3:
		game.global_vars[752] = 2000
	elif target_item.obj.stat_level_get( stat_level ) == 4:
		game.global_vars[752] = 4500
	elif target_item.obj.stat_level_get( stat_level ) == 5:
		game.global_vars[752] = 8000
	elif target_item.obj.stat_level_get( stat_level ) == 6:
		game.global_vars[752] = 12500
	elif target_item.obj.stat_level_get( stat_level ) == 7:
		game.global_vars[752] = 18000
	elif target_item.obj.stat_level_get( stat_level ) == 8:
		game.global_vars[752] = 24500
	elif target_item.obj.stat_level_get( stat_level ) == 9:
		game.global_vars[752] = 32000
	elif target_item.obj.stat_level_get( stat_level ) == 10:
		game.global_vars[752] = 40500
	elif target_item.obj.stat_level_get( stat_level ) == 11:
		game.global_vars[752] = 50000
	elif target_item.obj.stat_level_get( stat_level ) == 12:
		game.global_vars[752] = 60500
	elif target_item.obj.stat_level_get( stat_level ) == 13:
		game.global_vars[752] = 72000
	elif target_item.obj.stat_level_get( stat_level ) == 14:
		game.global_vars[752] = 84500
	elif target_item.obj.stat_level_get( stat_level ) == 15:
		game.global_vars[752] = 98000
	elif target_item.obj.stat_level_get( stat_level ) == 16:
		game.global_vars[752] = 112500
	elif target_item.obj.stat_level_get( stat_level ) == 17:
		game.global_vars[752] = 128000
	elif target_item.obj.stat_level_get( stat_level ) == 18:
		game.global_vars[752] = 144500
	elif target_item.obj.stat_level_get( stat_level ) == 19:
		game.global_vars[752] = 162000
	else:
		game.global_vars[752] = 180500


	target_item.obj.stat_base_set(stat_experience, game.global_vars[752])

	target_item.obj.object_script_execute( target_item.obj, 18 )
	
	End_Spell(spell)
	
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Reincarnation OnBeginRound"

def OnEndSpellCast( spell ):
	print "Reincarnation OnEndSpellCast"
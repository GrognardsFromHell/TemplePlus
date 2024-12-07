from toee import *
from scripts import End_Spell
from exclusions import *
from Co8 import *

def OnBeginSpellCast( spell ):
	print "Raise Dead OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Raise Dead OnSpellEffect"

	spell.duration = 0
	target_item = spell.target_list[0]

	raise_dead_exclusions = get_raise_dead_exclusions()
	if target_item.obj.name not in raise_dead_exclusions:
		#For aasumars and tieflings temporarily remove the outsider tag so raise dead will work
		prevType = 0
		if target_item.obj.is_category_type( mc_type_outsider ) and target_item.obj.is_category_subtype(mc_subtype_human):
			subrace = target_item.obj.stat_base_get(stat_subrace)
			if (subrace == human_subrace_aasumar or subrace == human_subrace_tiefling):
				prevType = target_item.obj.obj_get_int64(obj_f_critter_monster_category)
				target_item.obj.obj_set_int64(obj_f_critter_monster_category, mc_type_humanoid)

		target_item.obj.condition_add_with_args( 'sp-Raise Dead', spell.id, spell.duration, 0 )
		#target_item.partsys_id = game.particles( 'sp-Raise Dead', target_item.obj )
		
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
			
		#Restore the outsider tag if necessary
		if prevType != 0:
			target_item = spell.target_list[0]
			target_item.obj.obj_set_int64(obj_f_critter_monster_category, prevType)
			
		target_item.obj.stat_base_set(stat_experience, game.global_vars[752])

		target_item.obj.object_script_execute( target_item.obj, 18 )
	else:
		game.particles( 'Fizzle', target_item.obj )
		target_item.obj.float_mesfile_line ('mes\\spell.mes', 16020)
		target_item.obj.float_mesfile_line ('mes\\spell.mes', 16020)
		
	spell.spell_end( spell.id, 1 )

def OnBeginRound( spell ):
	spell.spell_end( spell.id, 1 )
	print "Raise Dead OnBeginRound"

def OnEndSpellCast( spell ):
	print "Raise Dead OnEndSpellCast"
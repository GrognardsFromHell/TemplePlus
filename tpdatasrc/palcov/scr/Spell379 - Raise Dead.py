from toee import *
from scripts import End_Spell
from exclusions import *

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

		target_item.obj.condition_add_with_args( 'sp-Raise Dead', spell.id, spell.duration, 0 )
		#target_item.partsys_id = game.particles( 'sp-Raise Dead', target_item.obj )

		xp_current_level, xp_previous_level = 0, 0
		for L in range (1, target_item.obj.stat_level_get(stat_level)+1):
			xp_previous_level = xp_current_level
			xp_current_level += (L-1)*1000
		xp_new = xp_current_level - ((xp_current_level - xp_previous_level) / 2)

		target_item.obj.stat_base_set(stat_experience, xp_new)
		target_item.obj.object_script_execute( target_item.obj, 18 )

	else:
		game.particles( 'Fizzle', target_item.obj )
		target_item.obj.float_mesfile_line ('mes\\spell.mes', 16020)
		target_item.obj.float_mesfile_line ('mes\\spell.mes', 16020)

	End_Spell(spell)  # why is this here? Marc wants to know.

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Raise Dead OnBeginRound"

def OnEndSpellCast( spell ):
	print "Raise Dead OnEndSpellCast"

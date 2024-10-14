from toee import *
from scripts import *


from utilities import float_num


def OnBeginSpellCast( spell ):
	print "Petrifying Gaze OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Petrifying Gaze OnSpellEffect"
	
	spell.dc = 13
	spell.duration = 100
	spell.caster_level = 10

	if spell.caster.name == 14295:  # basilisk
		spell.dc = 19
	if spell.caster.name == 14987:  # greater basilisk
		spell.dc = 21

	targets = list (game.obj_list_cone (spell.caster, OLC_CRITTERS, 60, -60, 120))
	for t in list(targets):
		if t.is_friendly(spell.caster) or t.d20_query_has_spell_condition(sp_Command):
			targets.remove(t)
	n = game.random_range (0, len(targets)-1)
	target = targets[n]

	if spell.caster.d20_query(Q_Critter_Is_Blinded):
		spell.caster.float_mesfile_line( 'mes\\spell.mes', 20019 )
		
	else:
		game.particles( 'sp-Shout', spell.caster )
		spell.caster.turn_towards(target)  # sometimes does not work

		if target.saving_throw( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, D20A_CAST_SPELL ):
			target.float_mesfile_line( 'mes\\spell.mes', 30001 )
			game.particles( 'Fizzle', target )

		else:
			target.float_mesfile_line( 'mes\\spell.mes', 30002 )
			target.float_mesfile_line( 'mes\\spell.mes', 20047 )
			target.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 4 )
			game.particles( "sp-Bestow Curse", target )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Petrifying Gaze OnBeginRound"

def OnEndSpellCast( spell ):
	print "Petrifying Gaze OnEndSpellCast"
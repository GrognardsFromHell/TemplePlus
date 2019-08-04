from toee import *

def OnBeginSpellCast( spell ):
	print "Lesser Vigor OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Lesser Vigor OnSpellEffect"

	spell.duration = 10 + min( 25, spell.caster.stat_level_get( spell.caster_class ) )

	target = spell.target_list[0]

	# Use any spell effect with a duration that you will not be using while under
	# the effects of vigor
	target.obj.condition_add_with_args( 'sp-Barkskin', spell.id, spell.duration, 0 )
	target.partsys_id = game.particles( 'sp-Cure Minor Wounds', target.obj )

	dice = dice_new( "1d1" )
	dice.bonus = 3
	target.obj.heal( OBJ_HANDLE_NULL, dice )
	target.obj.healsubdual( OBJ_HANDLE_NULL, dice )
	heal_count = 1
	heal_tick_time = 999
	if (not game.combat_is_active()):
		heal_tick_time = 6000
	while(heal_count < spell.duration):
		game.timeevent_add( heal_tick_greater_vigor, ( target.obj, dice ), (heal_count * heal_tick_time) )
		heal_count = heal_count + 1
	spell.target_list.remove_target(target.obj)
	spell.spell_end(spell.id)
	# end while

def OnBeginRound( spell ):
	print "Lesser Vigor OnBeginRound"

def OnEndSpellCast( spell ):
	print "Lesser Vigor OnEndSpellCast"

def heal_tick_greater_vigor( target, dice ):
	target.heal( OBJ_HANDLE_NULL, dice )
	target.healsubdual( OBJ_HANDLE_NULL, dice )

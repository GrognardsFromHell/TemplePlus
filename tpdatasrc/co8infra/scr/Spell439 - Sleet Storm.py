from toee import *

def OnBeginSpellCast( spell ):
	print "Sleet Storm OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

# restored original effect since the bug with infinite AoO was fixed

def OnSpellEffect( spell ):
	print "Sleet Storm OnSpellEffect"

	spell.duration = 1 * spell.caster_level

	# spawn one Sleet Storm scenery object
	sleet_storm_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	sleet_storm_obj.d20_status_init()
	sleet_storm_obj.set_initiative( caster_init_value )

	if spell.duration < 5:			##  added so NPC's can cast Sleet Storm
		spell.duration = 5		##  added so NPC's can cast Sleet Storm

	# put sp-Sleet Storm condition on obj
	sleet_storm_partsys_id = game.particles( 'sp-Sleet Storm', sleet_storm_obj )
	sleet_storm_obj.condition_add_with_args( 'sp-Sleet Storm', spell.id, spell.duration, 0, sleet_storm_partsys_id )
	#sleet_storm_obj.condition_add_arg_x( 3, sleet_storm_partsys_id )
	#objectevent_id = sleet_storm_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Sleet Storm OnBeginRound"

def OnEndSpellCast( spell ):
	print "Sleet Storm OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Sleet Storm OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Sleet Storm OnSpellStruck"
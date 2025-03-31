from toee import *

def OnBeginSpellCast( spell ):
	print "Stinking Cloud OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Stinking Cloud OnSpellEffect"

	spell.duration = 1 * spell.caster_level

	if spell.caster.name in [14259,14360]:    # Hezrou
		spell.dc = 24                         # Monster Manual
		spell.duration = 3
		
	# spawn one spell_object object
		spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Stinking Cloud condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Stinking Cloud', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Stinking Cloud', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

	if spell.caster.name in (14259,14360):  # Hezrou
		game.global_vars[369] = spell_obj_partsys_id

def OnBeginRound( spell ):
	print "Stinking Cloud OnBeginRound"

def OnEndSpellCast( spell ):
	print "Stinking Cloud OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Stinking Cloud OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Stinking Cloud OnSpellStruck"
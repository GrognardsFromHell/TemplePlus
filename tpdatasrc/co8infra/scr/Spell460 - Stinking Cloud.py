from toee import *

def OnBeginSpellCast( spell ):
	print "Stinking Cloud OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Stinking Cloud OnSpellEffect"

	
	if game.global_vars[451] & 2**8 != 0:
		spell.duration = game.random_range(1,3) + 1 # optional SC duration nerf: made 1d3 + 1
	else:
		spell.duration = 1 * spell.caster_level
	
	npc = spell.caster		

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

def OnBeginRound( spell ):
	print "Stinking Cloud OnBeginRound"

def OnEndSpellCast( spell ):
	print "Stinking Cloud OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Stinking Cloud OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Stinking Cloud OnSpellStruck"
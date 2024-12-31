from toee import *

def OnBeginSpellCast( spell ):
	print "Ice Storm OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Ice Storm OnSpellEffect"

	if spell.caster.name == 14977:  # Ice Devil
		spell.dc = 19               # 10 + 4 + 5
		spell.caster_level = 13

	spell.duration = 0

	# spawn one spell_object object
	spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )
	
	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Ice Storm condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Ice Storm', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Ice Storm', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Ice Storm OnBeginRound"

def OnEndSpellCast( spell ):
	print "Ice Storm OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Ice Storm OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Ice Storm OnSpellStruck"
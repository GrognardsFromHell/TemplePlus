from toee import *

def OnBeginSpellCast( spell ):
	print "Solid Fog OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Solid Fog OnSpellEffect"

	spell.duration = 100 * spell.caster_level

	# spawn one spell_object object
	spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Solid Fog condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Solid Fog', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Solid Fog', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Solid Fog OnBeginRound"

def OnEndSpellCast( spell ):
	print "Solid Fog OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Solid Fog OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Solid Fog OnSpellStruck"
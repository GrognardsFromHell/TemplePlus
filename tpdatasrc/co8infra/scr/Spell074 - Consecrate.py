from toee import *

def OnBeginSpellCast( spell ):
	print "Consecrate OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Consecrate OnSpellEffect"

	spell.duration = 1200 * spell.caster_level

	# spawn one spell_object object
	spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Consecrate condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Consecrate', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Consecrate', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Consecrate OnBeginRound"

def OnEndSpellCast( spell ):
	print "Consecrate OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Consecrate OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Consecrate OnSpellStruck"
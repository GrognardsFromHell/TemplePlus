from toee import *

def OnBeginSpellCast( spell ):
	print "Entangle OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Entangle OnSpellEffect"

	spell.duration = 10 * spell.caster_level
	outdoor_map_list = [5001, 5050, 5051, 5052, 5054, 5062, 5068, 5069, 5070, 5071, 5072, 5073, 5074, 5075, 5076, 5077, 5091, 5093, 5094, 5095, 5106, 5110, 5111, 5112, 5113]

	if game.leader.map in outdoor_map_list:

		# spawn one Entangle scenery object
		entangle_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc )

		# add to d20initiative
		caster_init_value = spell.caster.get_initiative()
		entangle_obj.d20_status_init()
		entangle_obj.set_initiative( caster_init_value )

		# put sp-Entangle condition on obj
		entangle_partsys_id = game.particles( 'sp-Entangle-Area', entangle_obj )
		entangle_obj.condition_add_with_args( 'sp-Entangle', spell.id, spell.duration, 0, entangle_partsys_id )
		#entangle_obj.condition_add_arg_x( 3, entangle_partsys_id )
		#objectevent_id = entangle_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Entangle OnBeginRound"

def OnEndSpellCast( spell ):
	print "Entangle OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Entangle OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Entangle OnSpellStruck"
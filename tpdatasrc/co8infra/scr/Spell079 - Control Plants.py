from toee import *
from utilities import  * 

def OnBeginSpellCast( spell ):
	print "Control Plants OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Control Plants OnSpellEffect"

#	Dar's level check no longer needed thanks to Spellslinger's dll fix
#	if spell.caster_class == 14:
#		if spell.spell_level < 3:#added to check for proper ranger slot level (darmagon)
#			spell.caster.float_mesfile_line('mes\\spell.mes', 16008)
#			spell.spell_end(spell.id)
#			return

	remove_list = []

	spell.duration = 60 * spell.caster_level

	# spawn one control_plants scenery object
	control_plants_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	control_plants_obj.d20_status_init()
	control_plants_obj.set_initiative( caster_init_value )

	# put sp-Control Plants condition on obj
	control_plants_partsys_id = game.particles( 'sp-Control Plants', control_plants_obj )
	control_plants_obj.condition_add_with_args( 'sp-Control Plants', spell.id, spell.duration, 0, control_plants_partsys_id )
	#control_plants_obj.condition_add_arg_x( 3, control_plants_partsys_id )
	#objectevent_id = control_plants_obj.condition_get_arg_x( 2 )

	# add wilderness_lord bonus to spell_caster
	spell.caster.condition_add_with_args( 'sp-Control Plants Tracking', spell.id, spell.duration, 0 )
	
def OnBeginRound( spell ):
	print "Control Plants OnBeginRound"

def OnEndSpellCast( spell ):
	print "Control Plants OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Control Plants OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Control Plants OnSpellStruck"
from toee import *

def OnBeginSpellCast( spell ):
	print "Fog Cloud OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Fog Cloud OnSpellEffect"

	spell.duration = 100 * spell.caster_level

	npc = spell.caster			##  added so NPC's can pre-buff
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active():
		spell.duration = 2000 * spell.caster_level

	# spawn one spell_object object
	spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	spell_obj.d20_status_init()
	spell_obj.set_initiative( caster_init_value )

	# put sp-Fog Cloud condition on obj
	spell_obj_partsys_id = game.particles( 'sp-Fog Cloud', spell_obj )
	spell_obj.condition_add_with_args( 'sp-Fog Cloud', spell.id, spell.duration, 0, spell_obj_partsys_id )
	#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
	#objectevent_id = spell_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Fog Cloud OnBeginRound"

def OnEndSpellCast( spell ):
	print "Fog Cloud OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Fog Cloud OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Fog Cloud OnSpellStruck"
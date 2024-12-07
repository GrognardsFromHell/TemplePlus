from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Silence OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-illusion-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Silence OnSpellEffect"

	npc = spell.caster			##  added so NPC's can use wand/potion/scroll
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL and spell.caster_level <= 0:
		spell.caster_level = 8

	if npc.name == 14425 and game.global_vars[711] == 1:
		spell.caster_level = 6
		spell.dc = 17

	spell.duration = 10 * spell.caster_level

	# test whether we targeted the ground or an object
	if spell.is_object_selected() == 1:
		target_item = spell.target_list[0]

		# allow Will saving throw to negate
		if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

			# saving throw successful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			game.particles( 'Fizzle', target_item.obj )
			spell.target_list.remove_target( target_item.obj )
		else:
			# put sp-Silence condition on target
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
			spell_obj_partsys_id = game.particles( 'sp-Silence', target_item.obj )
			target_item.obj.condition_add_with_args( 'sp-Silence', spell.id, spell.duration, 0, spell_obj_partsys_id )

	else:
		# spawn one spell_object object
		spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc )

		# add to d20initiative
		caster_init_value = spell.caster.get_initiative()
		spell_obj.d20_status_init()
		spell_obj.set_initiative( caster_init_value )

		# put sp-Silence condition on obj
		spell_obj_partsys_id = game.particles( 'sp-Silence', spell_obj )
		spell_obj.condition_add_with_args( 'sp-Silence', spell.id, spell.duration, 0, spell_obj_partsys_id )
		#spell_obj.condition_add_arg_x( 3, spell_obj_partsys_id )
		#objectevent_id = spell_obj.condition_get_arg_x( 2 )

def OnBeginRound( spell ):
	print "Silence OnBeginRound"

def OnEndSpellCast( spell ):
	print "Silence OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Silence OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Silence OnSpellStruck"
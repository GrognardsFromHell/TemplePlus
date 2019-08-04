from toee import *

def OnBeginSpellCast( spell ):
	print "Acid Fog OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Acid Fog OnSpellEffect"

	spell.duration = 10 * spell.caster_level

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

	# add monster to target list
	spell.num_of_targets = 1
	spell.target_list[0].obj = spell_obj


def OnBeginRound( spell ):
	print "Acid Fog OnBeginRound"
	damage_dice = dice_new( '2d6' )
	for obj in game.obj_list_vicinity(spell.target_list[0].obj.location,OLC_NPC):
		if (spell.target_list[0].obj.distance_to(obj) <= 20):
			obj.spell_damage( spell.caster, D20DT_ACID, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

def OnEndSpellCast( spell ):
	print "Acid Fog OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Acid Fog OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Acid Fog OnSpellStruck"
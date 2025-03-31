from toee import *

def OnBeginSpellCast( spell ):
	print "Cloudkill OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Cloudkill OnSpellEffect"

	spell.duration = 10 * spell.caster_level

	# added so you'll get awarded XP for the kill
	for target_item in spell.target_list:
		if not target_item.obj in game.leader.group_list():
			if target_item.obj.object_flags_get() & OF_INVULNERABLE == 0:
				target_item.obj.object_flag_set(OF_INVULNERABLE)
				target_item.obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )
				target_item.obj.object_flag_unset(OF_INVULNERABLE)
			else:
				target_item.obj.damage( game.leader , D20DT_UNSPECIFIED, dice_new( "1d1" ) )

	# spawn one Cloudkill scenery object
	cloudkill_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

	# add to d20initiative
	caster_init_value = spell.caster.get_initiative()
	cloudkill_obj.d20_status_init()
	cloudkill_obj.set_initiative( caster_init_value )

	# put sp-cloudkill_obj condition on obj
	cloudkill_obj_partsys_id = game.particles( 'sp-Cloudkill', cloudkill_obj )
	cloudkill_obj.condition_add_with_args( 'sp-Cloudkill', spell.id, spell.duration, 0, cloudkill_obj_partsys_id )

def OnBeginRound( spell ):
	print "Cloudkill OnBeginRound"

def OnEndSpellCast( spell ):
	print "Cloudkill OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Cloudkill OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Cloudkill OnSpellStruck"
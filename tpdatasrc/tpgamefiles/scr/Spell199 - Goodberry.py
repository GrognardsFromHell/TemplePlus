from toee import *

def OnBeginSpellCast( spell ):
	print "Goodberry OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Goodberry OnSpellEffect"

	dice = dice_new( '2d4' )
	num_of_berries = dice.roll()

	spell.duration = 0
	target_item = spell.target_list[0]
	goodberry_proto_id = 8001

	game.particles( 'sp-Goodberry', target_item.obj )

	for index in range( num_of_berries ):
		item_obj = game.obj_create( goodberry_proto_id, spell.target_loc )
		item_obj.item_flag_set(OIF_IDENTIFIED)
		spell.caster.item_get( item_obj )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Goodberry OnBeginRound"

def OnEndSpellCast( spell ):
	print "Goodberry OnEndSpellCast"
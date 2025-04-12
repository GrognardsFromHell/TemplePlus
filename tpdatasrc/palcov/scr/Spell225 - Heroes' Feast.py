from toee import *

def OnBeginSpellCast( spell ):
	print "Heroes' Feast OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Heroes' Feast OnSpellEffect"

	spell.duration = 7200

	for target_item in spell.target_list:
		return_val1 = target_item.obj.condition_add_with_args( 'sp-Aid', spell.id, spell.duration, 1 )
		if return_val1 == 1:
			target_item.partsys_id = game.particles( 'sp-Aid', target_item.obj )

	for target_item in spell.target_list:
		return_val2 = target_item.obj.condition_add_with_args( 'sp-Neutralize Poison', spell.id, spell.duration, 1 )
		if return_val2 == 1:
			game.particles( 'sp-Neutralize Poison', target_item.obj )

	for target_item in spell.target_list:
		#return_val3 = target_item.obj.condition_add_with_args( 'sp-Remove Disease', spell.id, 0, 1 )
		#if return_val3 == 1:
		#	game.particles( 'sp-Remove Disease', target_item.obj )
		# Removed this since it removes the target object, as the Remove Disease spell is instantaneous
		# Instead, using S_Remove_Disease
		# Not perfect since it doesn't cure Vrock Spores...
		target_item.obj.d20_send_signal(S_Remove_Disease)

	for target_item in spell.target_list:
		return_val4 = target_item.obj.condition_add_with_args( 'sp-Remove Fear', spell.id, spell.duration, 0 )
		if return_val4 == 1:
			target_item.partsys_id = game.particles( 'sp-Remove Fear', target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Heroes' Feast OnBeginRound"

def OnEndSpellCast( spell ):
	print "Heroes' Feast OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):
	print "Slow OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Slow OnSpellEffect"

	remove_list = []

	spell.duration = 1 * spell.caster_level

	for target_item in spell.target_list:

		if not target_item.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
	
			# saving throw unsuccessful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			return_val = target_item.obj.condition_add_with_args( 'sp-Slow', spell.id, spell.duration, 1 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Slow', target_item.obj )
			else:
				# condition was preempted by another (haste, other slow, ...)
				remove_list.append(target_item.obj)

		else:
	
			# saving throw successful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			game.particles( 'Fizzle', target_item.obj )
			remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Slow OnBeginRound"

def OnEndSpellCast( spell ):
	print "Slow OnEndSpellCast"

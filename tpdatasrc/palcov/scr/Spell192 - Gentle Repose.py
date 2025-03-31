from toee import *

def OnBeginSpellCast( spell ):
	print "Gentle Repose OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Gentle Repose OnSpellEffect"

	target = spell.target_list[0]

	if not target.obj.is_friendly( spell.caster ):

		if not target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id):
			# saving throw unsuccessful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
			game.particles( 'sp-Inflict Light Wounds', target.obj )
			target.obj.float_mesfile_line( 'mes\\spell.mes', 192 )
			x = target.obj.obj_get_int(obj_f_critter_flags2)
			x = x | 64
			target.obj.obj_set_int(obj_f_critter_flags2, x)
			# why is this here? hommlet farm animals killed counter, marc.
			# game.global_vars[900] = target.obj.obj_get_int(obj_f_critter_flags2)

		else:

			# saving throw successful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			game.particles( 'Fizzle', target.obj )

	else:
		game.particles( 'sp-Inflict Light Wounds', target.obj )
		target.obj.float_mesfile_line( 'mes\\spell.mes', 192 )

		x = target.obj.obj_get_int(obj_f_critter_flags2)
		x = x | 64
		target.obj.obj_set_int(obj_f_critter_flags2, x)

	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Gentle Repose OnBeginRound"

def OnEndSpellCast( spell ):
	print "Gentle Repose OnEndSpellCast"
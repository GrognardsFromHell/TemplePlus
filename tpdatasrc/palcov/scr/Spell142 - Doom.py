from toee import *

def OnBeginSpellCast( spell ):
	print "Doom OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Doom OnSpellEffect"

	spell.duration = 10 * spell.caster_level

	target = spell.target_list[0]

	if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):

		# allow Will saving throw to negate
		if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
			# saving throw successful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

			game.particles( 'Fizzle', target.obj )
			spell.target_list.remove_target( target.obj )

		else:
			# saving throw unsuccessful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

			# HTN - apply condition DOOM
			target.obj.condition_add_with_args( 'sp-Doom', spell.id, spell.duration, 0 )
			target.partsys_id = game.particles( 'sp-Doom', target.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Doom OnBeginRound"

def OnEndSpellCast( spell ):
	print "Doom OnEndSpellCast"
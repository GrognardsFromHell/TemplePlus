from toee import *
from utilities import  *
from stench import neutraliseStench

def OnBeginSpellCast( spell ):
	print "Neutralize Poison OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Neutralize Poison OnSpellEffect"
	if spell.caster_class == 13: #added to check for proper paladin slot level (darmagon)
		if spell.spell_level < 4:
			spell.caster.float_mesfile_line('mes\\spell.mes', 16008)
			spell.spell_end(spell.id)
			return
	if spell.caster_class == 14:
		if spell.spell_level < 3:#added to check for proper ranger slot level (darmagon)
			spell.caster.float_mesfile_line('mes\\spell.mes', 16008)
			spell.spell_end(spell.id)
			return


	spell.duration = 1

	target = spell.target_list[0]

	if target.obj.is_friendly( spell.caster ):

		# Neutralise any Hezrou Stench effects.
		neutraliseStench(target.obj, 600 * spell.caster_level)

		#target.partsys_id = game.particles( 'sp-Neutralize Poison', target.obj )
		game.particles( 'sp-Neutralize Poison', target.obj ) # the particles get ended when condition_add_with_args is applied so we separate them from the target.partsys_id
		target.obj.condition_add_with_args( 'sp-Neutralize Poison', spell.id, spell.duration, 0 )
		

	elif not target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):

		# saving throw unsuccesful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		# Neutralise any Hezrou Stench effects.
		neutraliseStench(target.obj, )

		#target.partsys_id = game.particles( 'sp-Neutralize Poison', target.obj )
		game.particles( 'sp-Neutralize Poison', target.obj )
		target.obj.condition_add_with_args( 'sp-Neutralize Poison', spell.id, spell.duration, 0 )
		

	else:
		print "Saving throw successful: " + str(target)
		# saving throw successful
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

		game.particles( 'Fizzle', obj )
		spell.target_list.remove_target( target.obj )

	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Neutralize Poison OnBeginRound"

def OnEndSpellCast( spell ):
	print "Neutralize Poison OnEndSpellCast"
from toee import *

def OnBeginSpellCast( spell ):

	print "Petrifying Breath OnBeginSpellCast"
	print "spell.target_list=",spell.target_list
	print "\nspell.caster= ",spell.caster
	print " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Petrifying Breath OnSpellEffect"

	target_list = []

	spell.dc = 19
	spell.duration = 100
	spell.caster_level = 10

#	dam = dice_new( '1d6' )
#	dam.number = min( 15, spell.caster_level )

	game.particles( 'sp-Cone of Cold', spell.caster )
	
	# get all targets in a 25ft + 5ft/2levels cone (60')
#	range = 25 + 5 * int(spell.caster_level/2)
	range = 60
	target_list = list(game.obj_list_cone( spell.caster, OLC_CRITTERS, range, -30, 60 ))
#	print >> efile, "spell range= ", range, "\n"
	target_list.remove(spell.caster)
#	print >> efile, "target list: ", target_list, "\n"
	for obj in target_list:
		if obj.name == 14309:
			game.global_flags[811] = 0
		if obj.saving_throw( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, D20A_CAST_SPELL ):
			# saving throw successful
			obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

		else:
			# saving throw unsuccessful
			obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
			# HTN - apply condition HALT (Petrifyed)
			obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 4 )
			game.particles( 'sp-Bestow Curse', obj )

	spell.target_list.remove_list( target_list )
	spell.spell_end( spell.id )
#	efile.close()
	
def OnBeginRound( spell ):
	print "Petrifying Breath OnBeginRound"

def OnEndSpellCast( spell ):
	print "Petrifying Breath OnEndSpellCast"
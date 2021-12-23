from toee import *

def OnBeginSpellCast( spell ):
	print "Cone of Cold OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Cone of Cold OnSpellEffect"

	remove_list = []

	dam = dice_new( '1d6' )
	dam.number = min( 15, spell.caster_level )

	game.particles( 'sp-Cone of Cold', spell.caster )

	# get all targets in a 25ft + 2ft/level cone (60')
	for target_item in spell.target_list:

		if target_item.obj.reflex_save_and_damage( spell.caster, spell.dc, D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_COLD, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id ) > 0:
			# saving throw successful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		else:
			# saving throw unsuccessful
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Cone of Cold OnBeginRound"

def OnEndSpellCast( spell ):
	print "Cone of Cold OnEndSpellCast"
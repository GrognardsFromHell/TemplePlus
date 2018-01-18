from toee import *

def OnBeginSpellCast( spell ):
	print "Frozen Breath OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-evocation-conjure", spell.caster )

def	OnSpellEffect ( spell ):
	print "Frozen Breath OnSpellEffect"

	remove_list = []

	dam = dice_new( '1d6' )
	dam.number = spell.spell_level

	if dam.number > 6:
		dam.number = 6
	game.particles( 'sp-Cone of Cold', spell.caster )

	npc = spell.caster

	spell.dc = spell.dc + 5

	if npc.name == 14999: ## Old White Dragon
		dam.number = 8
		spell.dc = 27

#	range = 25 + 5 * int(spell.caster_level/2)
	range = 60
	target_list = list(game.obj_list_cone( spell.caster, OLC_CRITTERS, range, -30, 60 ))
	target_list.remove(spell.caster)
	for obj in target_list:
		if obj.reflex_save_and_damage( spell.caster, spell.dc, 
D20_Save_Reduction_Half, D20STD_F_NONE, dam, D20DT_COLD, D20DAP_UNSPECIFIED, 
D20A_CAST_SPELL, spell.id ) > 0:
			# saving throw successful
			obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		else:
			# saving throw unsuccessful
			obj.float_mesfile_line( 'mes\\spell.mes', 30002 )



	spell.target_list.remove_list( remove_list )
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Frozen Breath OnBeginRound"

def OnEndSpellCast( spell ):
	print "Frozen Breath OnEndSpellCast"

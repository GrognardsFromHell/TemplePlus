from toee import *

def OnBeginSpellCast( spell ):
	print "Read Magic OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-divination-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Read Magic OnSpellEffect"

	spell.duration = 0

	target_item = spell.target_list[0]

	if target_item.obj.name == 9900:
		game.particles( 'sp-Bestow Curse', spell.caster )
		game.sound(6562,1)  # sp_Bestow_Curse.wav
		target_item.obj.destroy()
		for x in (1,2,3):
			spell.caster.float_mesfile_line( 'mes\\spell.mes', 30020 )
			spell.caster.float_mesfile_line( 'mes\\spell.mes', 30021 )
		cha = spell.caster.stat_base_get(stat_charisma)
		spell.caster.stat_base_set(stat_charisma, cha - 1)

	else:
		game.particles( 'sp-Read Magic', spell.caster )
		target_item.obj.item_flag_set( OIF_IDENTIFIED )

	spell.target_list.remove_target( target_item.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Read Magic OnBeginRound"

def OnEndSpellCast( spell ):
	print "Read Magic OnEndSpellCast"
from toee import *
from scripts import *


from utilities import float_num


def OnBeginSpellCast( spell ):
	print "Petrifying Gaze OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-enchantment-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Petrifying Gaze OnSpellEffect"
	
	spell.dc = 13
	spell.duration = 100
	spell.caster_level = 10

	if spell.caster.name == 14295:  # basilisk
		spell.dc = 19
	if spell.caster.name == 14987:  # greater basilisk
		spell.dc = 21

	target = spell.target_list[0]

	num = 0
	num2 = 0
	target_list = list(game.obj_list_cone( spell.caster, OLC_CRITTERS, 60, -30, 60 ))
#	print >> efile, "spell range= ", range, "\n"
	target_list.remove(spell.caster)
#	print >> efile, "target list: ", target_list, "\n"
	for obj in target_list:
		if not obj.is_friendly(spell.caster):
			num = num + 1
		else:
			target_list.remove(obj)
	num = game.random_range(1,num)
	for obj in target_list:
		num2 = num2 + 1
		if num == num2:
			target.obj = obj

#	print >> efile, "target.obj: ", target.obj, "\n"
		

	if spell.caster.d20_query(Q_Critter_Is_Blinded):
		spell.caster.float_mesfile_line( 'mes\\spell.mes', 20019 )
	elif target.obj.saving_throw( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, D20A_CAST_SPELL ):
		game.particles( 'sp-Shout', spell.caster )
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		game.particles( 'Fizzle', target.obj )
		spell.target_list.remove_target( target.obj )	
	else:
		game.particles( 'sp-Shout', spell.caster )
		target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
		# HTN - apply condition HALT (Petrifyed)
		target.obj.condition_add_with_args( 'sp-Command', spell.id, spell.duration, 4 )
		game.particles( "sp-Bestow Curse", target.obj )

	spell.spell_end( spell.id )
#	efile.close()


def OnBeginRound( spell ):
	print "Petrifying Gaze OnBeginRound"

def OnEndSpellCast( spell ):
	print "Petrifying Gaze OnEndSpellCast"
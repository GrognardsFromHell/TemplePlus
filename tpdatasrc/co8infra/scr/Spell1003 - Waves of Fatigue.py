from toee import *

def OnBeginSpellCast( spell ):
	print "Waves of Fatigue OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Waves of Fatigue OnSpellEffect"

	remove_list = []

	game.particles( 'sp-Waves of Fatigue', spell.caster )

	for target_item in spell.target_list:
		wrongType = target_item.obj.is_category_type(mc_type_undead) or target_item.obj.is_category_type(mc_type_construct) or target_item.obj.is_category_type(mc_type_ooze) or target_item.obj.is_category_type(mc_type_plant)
		if wrongType:
			target_item.obj.float_text_line("Fatigue Immunity")
		else:
			hasExhaust = target_item.obj.d20_query("Exhausted")
			hasFatigue = target_item.obj.d20_query("Fatigued")
			if hasFatigue or hasExhaust:
				if hasExhaust:
					target_item.obj.float_text_line("Already Exhausted")
				else:
					target_item.obj.float_text_line("Already Fatigued")
			else:
				target_item.obj.condition_add_with_args("FatigueExhaust", 0, -1, 0, 1, 0, 0) #Lasts Until Rest
		remove_list.append( target_item.obj )

	spell.target_list.remove_list( remove_list )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Waves of Fatigue OnBeginRound"

def OnEndSpellCast( spell ):
	print "Waves of Fatigue OnEndSpellCast"
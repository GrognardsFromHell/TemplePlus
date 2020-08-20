from toee import *

def OnBeginSpellCast( spell ):
	print "Harm OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Harm OnSpellEffect"

	target = spell.target_list[0]

	# check if target is friendly (willing target)
	if target.obj.is_friendly( spell.caster ):
		if not target.obj.is_category_type( mc_type_undead ):
			# Harm target
			target.obj.condition_add_with_args( 'sp-Harm', spell.id, spell.duration, 0 )
		else:
			# Heal undead
			target.obj.condition_add_with_args( 'sp-Heal', spell.id, spell.duration, 0 )

	else:
		attack_result = spell.caster.perform_touch_attack(target.obj, 1)
		if attack_result & D20CAF_HIT:
			# check if target is undead
			if not target.obj.is_category_type( mc_type_undead ):
				target.obj.condition_add_with_args( 'sp-Harm', spell.id, spell.duration, 0 )
			else:
				target.obj.condition_add_with_args( 'sp-Heal', spell.id, spell.duration, 0 )

	game.particles( 'sp-Harm', target.obj )
	
	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Harm OnBeginRound"

def OnEndSpellCast( spell ):
	print "Harm OnEndSpellCast"

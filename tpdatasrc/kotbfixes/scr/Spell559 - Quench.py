from toee import *

def OnBeginSpellCast( spell ):
	print "Quench OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Quench OnSpellEffect"

	# duration is 1d4 hours if implementing 2nd part of quench
	spell.duration = 0
	target_item = spell.target_list[0]

	damage_dice = dice_new( '1d6' )
	damage_dice.number = min( spell.caster_level, 15 )

	target_item.partsys_id = game.particles( 'sp-Quench', target_item.obj )

	# only damages fire elementals
	if (target_item.obj.is_category_type( mc_type_elemental )) and (target_item.obj.is_category_subtype( mc_subtype_fire )):

		# no save, full damage
		target_item.obj.spell_damage( spell.caster, D20DT_MAGIC, damage_dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )
		spell.target_list.remove_target( target_item.obj )

	else:

		# allowing targeting of creatures? 2nd part of quench
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 31013 )
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Quench OnBeginRound"

def OnEndSpellCast( spell ):
	print "Quench OnEndSpellCast"
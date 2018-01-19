from toee import *

def OnBeginSpellCast( spell ):
	print "Frog Tongue OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Frog Tongue OnSpellEffect"

	# it takes 1 round to pull the target to the frog (normally)
	spell.duration = 0

	target_item = spell.target_list[0]

	# if the target is larger than the frog, it takes 2 turns to "pull" the target in
	if target_item.obj.get_size > spell.caster.get_size:
		spell.duration = 1

	has_freedom = 0
	if target_item.obj.d20_query(Q_Critter_Has_Freedom_of_Movement):
		has_freedom = 1
	ranged_touch_res = spell.caster.perform_touch_attack( target_item.obj )
	if (ranged_touch_res & D20CAF_HIT) and not has_freedom:

		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 21000 )

		# hit
		#target_item.obj.condition_add_with_args( 'sp-Frog Tongue', spell.id, spell.duration, 0 )
		spell.caster.condition_add_with_args( 'sp-Frog Tongue', spell.id, spell.duration, 0 )
		target_item.partsys_id = game.particles( 'sp-Frog Tongue', target_item.obj )

	else:

		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 21001 )

		spell.caster.anim_callback( ANIM_CALLBACK_FROG_FAILED_LATCH )

		# missed
		if not (ranged_touch_res & D20CAF_HIT):
			target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30007 )

		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	if spell.caster == OBJ_HANDLE_NULL:
		spell.spell_end(spell.id, 1)
	elif spell.caster.is_unconscious():
		spell.spell_end(spell.id, 1)
	print "Frog Tongue OnBeginRound"

def OnEndSpellCast( spell ):
	print "Frog Tongue OnEndSpellCast"

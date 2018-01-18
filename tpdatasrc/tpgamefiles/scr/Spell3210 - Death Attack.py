from toee import *

def OnBeginSpellCast( spell ):
	print "Death Attack OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Assassin Death Attack OnSpellEffect"

	spell.duration = 0

	target = spell.target_list[0]

	if target.obj.d20_query(Q_Critter_Is_Immune_Critical_Hits):
		return

	target.obj.float_text_line("Target Marked", tf_red)

	target.obj.condition_add_with_args( 'Death Attack Target', spell.id, spell.duration, 0 )



def OnBeginRound( spell ):
	print "Assassin Death Attack OnBeginRound"
	target = spell.target_list[0]
	if target.obj == OBJ_HANDLE_NULL:
		return
	caster = spell.caster
	#target.obj.d20_send_signal("Target Study Round", spell.id)
	roundsStudied = target.obj.d20_query_with_data("Death Attack Target Rounds Studied", spell.id)
	if roundsStudied == 0:
		# means this has been removed externally or failed to apply
		spell.spell_end(spell.id, 1)
		return

	if roundsStudied >= 6:
		# end the studiment
		caster.float_text_line("Target Study Expired", tf_red)
		target.obj.d20_send_signal("Death Attack Target End", spell.id)
		spell.spell_end(spell.id, 1)
	elif roundsStudied == 3:
		caster.float_text_line("Target Studied!")
		print "Target study finished!"
	elif roundsStudied < 3:
		caster.float_text_line("Target Studied " + str(roundsStudied) + " Rounds", tf_white)



def OnEndSpellCast( spell ):
	print "Assassin Death Attack OnEndSpellCast"
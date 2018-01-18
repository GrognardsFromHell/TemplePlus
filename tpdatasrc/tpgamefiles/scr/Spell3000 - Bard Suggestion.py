from toee import *

def OnBeginSpellCast( spell ):
	print "Suggestion OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Suggestion OnSpellEffect"

	spell.duration = 600 * spell.caster_level
	#print "Duration: " + str(spell.duration)

	remove_list = []
	for tgt in spell.target_list:
		obj = tgt.obj
		if obj == OBJ_HANDLE_NULL:
			continue
		if not obj.d20_query(Q_Helpless): # Fascinated creatures are rendered helpless
			remove_list.append(obj)
			obj.float_text_line("Not Fascinated!", tf_red)
			continue
		bard_lvl = spell.caster.stat_level_get(stat_level_bard)
		bard_cha_mod = (spell.caster.stat_level_get(stat_charisma) - 10) /2
		if obj == spell.caster or obj.saving_throw( 10 + bard_lvl/2 + bard_cha_mod, D20_Save_Will, D20STD_F_NONE, spell.caster, D20A_BARDIC_MUSIC ):
			remove_list.append(obj)
			obj.float_mesfile_line('mes\\spell.mes', 30001)
		else:
			obj.d20_send_signal(S_Bardic_Music_Completed)
			obj.condition_add_with_args("Bard Suggestion", 600 * bard_lvl, 0, spell.id)
			obj.float_mesfile_line('mes\\spell.mes', 30002)

	spell.target_list.remove_list(remove_list)

	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")


def OnBeginRound( spell ):
	obj = spell.begin_round_obj
	print "Suggestion OnBeginRound for " + str(obj)
	#print "Duration Remaining: " + str(spell.duration_remaining)

	remove_list = []

	if obj == OBJ_HANDLE_NULL or obj == spell.caster:
		return
	if obj.is_unconscious():
		remove_list.append(obj)

	spell.target_list.remove_list(remove_list)
	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")
	return


def OnEndSpellCast( spell ):
	print "Suggestion OnEndSpellCast"
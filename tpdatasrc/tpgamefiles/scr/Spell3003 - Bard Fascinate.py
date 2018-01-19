from toee import *

def OnBeginSpellCast( spell ):
	print "Fascinate OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Fascinate OnSpellEffect"

	spell.duration = spell.caster_level
	print "Duration: " + str(spell.duration)

	remove_list = []
	for tgt in spell.target_list:
		obj = tgt.obj
		if obj == OBJ_HANDLE_NULL:
			continue
		roll = game.random_range(1,20)
		perfLvl = spell.caster.skill_level_get(skill_perform)
		if obj == spell.caster or obj.saving_throw( perfLvl + roll, D20_Save_Will, D20STD_F_NONE, spell.caster, D20A_BARDIC_MUSIC ):
			remove_list.append(obj)
		else:
			obj.condition_add("Fascinate")

	spell.target_list.remove_list(remove_list)
	#target.obj.float_text_line("Target Marked", tf_red)
	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")


def OnBeginRound( spell ):
	obj = spell.begin_round_obj
	print "Fascinate OnBeginRound for " + str(obj)
	#print "Duration Remaining: " + str(spell.duration_remaining)

	cur_song = spell.caster.d20_query("Bardic Music Type")
	if  cur_song != BM_FASCINATE and cur_song != BM_SUGGESTION: #
		spell.spell_end(spell.id, 1)
		#print "Ending Fascinate because of changed music type"
		return


	remove_list = []

	if obj == OBJ_HANDLE_NULL or obj == spell.caster:
		return

	roll = game.random_range(1,20)
	perfLvl = spell.caster.skill_level_get(skill_perform)
	if ( game.combat_is_active() and obj.saving_throw( perfLvl + roll, D20_Save_Will, D20STD_F_NONE, spell.caster, D20A_BARDIC_MUSIC ) ) or spell.caster.distance_to(obj) > 90 or (not obj.d20_query(Q_Helpless)):

		#print "Removing target " + str(obj)
		#if not obj.d20_query(Q_Helpless):
		#	print "Because it wasn't helpless!"
		remove_list.append(obj)
	else:
		obj.condition_add("Fascinate")

	spell.target_list.remove_list(remove_list)
	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")
	return


def OnEndSpellCast( spell ):
	print "Fascinate OnEndSpellCast"
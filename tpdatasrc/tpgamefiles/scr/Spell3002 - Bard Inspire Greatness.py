from toee import *

def OnBeginSpellCast( spell ):
	print "Inspire Greatness OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Inspire Greatness OnSpellEffect"

	spell.duration = spell.caster_level
	#print "Duration: " + str(spell.duration)

	remove_list = []
	for tgt in spell.target_list:
		obj = tgt.obj
		if obj == OBJ_HANDLE_NULL:
			continue
		if spell.caster.distance_to(obj) > 90:
			# print "Removing target " + str(obj)
			remove_list.append(obj)
		else:
			obj.condition_add("Greatness")

	spell.target_list.remove_list(remove_list)
	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")


def OnBeginRound( spell ):
	obj = spell.begin_round_obj
	print "Inspire Greatness OnBeginRound for " + str(obj)
	#print "Duration Remaining: " + str(spell.duration_remaining)

	if spell.caster.d20_query("Bardic Music Type") != 6: # BM_INSPIRE_GREATNESS
		spell.spell_end(spell.id, 1)
		return


	remove_list = []

	if obj == OBJ_HANDLE_NULL:
		return

	if spell.caster.distance_to(obj) > 30:
		#print "Removing target " + str(obj)
		remove_list.append(obj)
	else:
		bonusRounds = spell.caster.d20_query("Bardic Ability Duration Bonus")
		obj.condition_add("Greatness", bonusRounds+5, 0, 0, 0)

	spell.target_list.remove_list(remove_list)
	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)
		spell.caster.d20_send_signal("Bardic Music End")
	return


def OnEndSpellCast( spell ):
	print "Inspire Greatness OnEndSpellCast"
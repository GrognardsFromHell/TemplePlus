from toee import *

def OnBeginSpellCast( spell ):
	print "Hail of Arrows OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level

def	OnSpellEffect( spell ):
	print "Hail of Arrows OnSpellEffect"

	spell.duration = 0
	#print "Duration: " + str(spell.duration)

	remove_list = []
	for tgt in spell.target_list:
		obj = tgt.obj
		if obj == OBJ_HANDLE_NULL:
			continue

	spell.target_list.remove_list(remove_list)

	if spell.num_of_targets == 0:
		#print "num of targets == 0"
		spell.spell_end(spell.id)


def OnBeginRound( spell ):
	obj = spell.begin_round_obj
	print "Hail of Arrows OnBeginRound for " + str(obj)


def OnEndSpellCast( spell ):
	print "Hail of Arrows OnEndSpellCast"
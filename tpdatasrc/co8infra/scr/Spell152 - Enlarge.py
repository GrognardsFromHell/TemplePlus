from toee import *
from Co8 import *

import _include
from co8Util import size
from co8Util.PersistentData import *
from co8Util.ObjHandling import *

ENLARGE_KEY = "Sp152_Enlarge_Activelist"

def OnBeginSpellCast( spell ):
	print "Enlarge OnBeginSpellCast\n"
	game.particles( "sp-transmutation-conjure", spell.caster )


def OnSpellEffect(spell):
	print "Enlarge OnSpellEffect\n"
	spell.duration = 10 * spell.caster_level

	to_remove = []

	for target_item in spell.target_list:
		target = target_item.obj

		# enlarger PERSON only
		if target.is_category_type(mc_type_humanoid) != 1:
			to_remove.append(target)
			target.float_mesfile_line('mes\\spell.mes', 30000)
			target.float_mesfile_line('mes\\spell.mes', 31004)

			game.particles('Fizzle', target)
			continue

		# testing for reduce because I don't think you can fort save against
		# the dispel
		if not (target.d20_query_has_spell_condition(sp_Reduce) or target.is_friendly(spell.caster)):
			fort = D20_Save_Fortitude
			if target.saving_throw_spell(spell.dc, fort, D20STD_F_NONE, spell.caster, spell.id):
				target.float_mesfile_line('mes\\spell.mes', 30001)

				game.particles('Fizzle', target)
				to_remove.append(target)
				continue
			else:
				# saving throw unsuccesful
				target.float_mesfile_line('mes\\spell.mes', 30002)

		return_val = target.condition_add_with_args('sp-Enlarge', spell.id, spell.duration, 0)
		if return_val == 1:
			target_item.partsys_id = game.particles('sp-Enlarge', target)
		else:
			# sp-Enlarge not applied, probably dispelled sp-Reduce
			to_remove.append(target)

	spell.target_list.remove_list(to_remove)
	spell.spell_end(spell.id)

def OnBeginRound(spell):
	print "Enlarge OnBeginRound"
	UndoCo8(spell)
	return


def OnEndSpellCast(spell):
	#strangely enough the target_list gets zeroed before entering this, not after
	print "Enlarge OnEndSpellCast"
	UndoCo8(spell)
   
def UndoCo8(spell):
	activeList = Co8PersistentData.getData(ENLARGE_KEY)
	if isNone(activeList): return

	for entry in activeList:
		spellID, target = entry
		targetObj = refHandle(target)
		if spellID == spell.id:
			weap_too_big(targetObj)
			size.resetSizeCategory(targetObj)
			activeList.remove(entry)
			#no more active spells
			if len(activeList) == 0:
				Co8PersistentData.removeData(ENLARGE_KEY)
				break
			#save new activeList
			Co8PersistentData.setData(ENLARGE_KEY, activeList)
			break


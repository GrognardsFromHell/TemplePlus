from toee import *

debug = False
def Debug(msg, *extra):
	if not debug: return

	print msg
	for ex in extra:
		print ex

def OnBeginSpellCast( spell ):
	Debug("Reduce Animal OnBeginSpellCast")
	game.particles( "sp-transmutation-conjure", spell.caster )

def	OnSpellEffect( spell ):
	Debug("Reduce Animal OnSpellEffect")

	spell.duration = 600 * spell.caster_level

	to_remove = []
	for target_item in spell.target_list:
		target = target_item.obj
		if not target.is_friendly(spell.caster):
			# this should be impossible from the targeting, but just in case,
			# Reduce Animal only affects willing animals
			target.float_mesfile_line('mes\\spell.mes', 30003)
			to_remove.append(target)
			continue

		if target.is_category_type(mc_type_animal) != 1:
			target.float_mesfile_line('mes\\spell.mes', 30000)
			target.float_mesfile_line('mes\\spell.mes', 31002)

			game.particles('Fizzle', target)
			to_remove.append(target)
			continue

		target.condition_add_with_args('sp-Reduce Animal', spell.id, spell.duration, 0)
		target_item.partsys_id = game.particles('sp-Reduce Animal', target)

	spell.target_list.remove_list(to_remove)
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	Debug("Reduce Animal OnBeginRound")

def OnEndSpellCast( spell ):
	Debug("Reduce Animal OnEndSpellCast")

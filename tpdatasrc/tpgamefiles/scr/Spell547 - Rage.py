from toee import *

debug = False

def debug_print(*args):
	if not debug: return

	out = ""
	for arg in args:
		"{}{}".format(out,arg)

	print out

def OnBeginSpellCast(spell):
	debug_print("Rage OnBeginSpellCast")
	debug_print("spell.target_list=", spell.target_list)
	debug_print("spell.caster=", spell.caster, " caster.level= ", spell.caster_level)

def	OnSpellEffect( spell ):
	debug_print("Rage OnSpellEffect")

	remove_list = []

	spell.duration = spell.caster_level

	for target_item in spell.target_list:
		target = target_item.obj

		if target.is_friendly(spell.caster):
			target.condition_add_with_args('sp-Rage', spell.id, spell.duration, 0)
			target_item.partsys_id = game.particles('sp-Rage', target_item.obj)
		else:
			game.particles('Fizzle', target)
			remove_list.append(target)

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	debug_print("Rage OnBeginRound")

def OnEndSpellCast( spell ):
	debug_print("Rage OnEndSpellCast")

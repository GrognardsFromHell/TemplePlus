from toee import *

debug = False

def debug_print(*args):
	if not debug: return

	out = ""
	for arg in args:
		"{}{}".format(out, arg)

	print out

def OnBeginSpellCast( spell ):
	debug_print("Ghoul Touch OnBeginSpellCast")
	debug_print("spell.target_list=", spell.target_list)
	debug_print("spell.caster=", spell.caster, " caster.level= ", spell.caster_level)
	game.particles("sp-necromancy-conjure", spell.caster)

def	OnSpellEffect(spell):
	debug_print("Ghoul Touch OnSpellEffect")

	spell.duration = 0
	target_item = spell.target_list[0]
	target = target_item.obj

	target.condition_add_with_args('sp-Ghoul Touch', spell.id, spell.duration, 0)
	target_item.partsys_id = game.particles('sp-Ghoul Touch-HELD', target)

def OnBeginRound(spell):
	debug_print("Ghoul Touch OnBeginRound")

def OnEndSpellCast(spell):
	debug_print("Ghoul Touch OnEndSpellCast")

def OnAreaOfEffectHit(spell):
	debug_print("Ghoul Touch OnAreaOfEffectHit")

def OnSpellStruck(spell):
	debug_print("Ghoul Touch OnSpellStruck")

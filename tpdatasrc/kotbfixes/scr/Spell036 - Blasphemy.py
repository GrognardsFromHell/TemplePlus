from toee import *
from alignment_words import *

def OnBeginSpellCast(spell):
	return

def ApplyDaze(spell, target):
	target.condition_add_with_args('sp-Daze', 0, 1, 0)

def ApplyWeakness(spell, target):
	dur = dice_new('2d4').roll()
	penalty = dice_new('2d6').roll()
	target.condition_add_with_args('Weakness', dur, 0, penalty)

def ApplyParalyzed(spell, target):
	dur = 10 * dice_new('1d10').roll()
	target.condition_add_with_args('Paralyzed', dur, spell.dc)

effects = [
		(0, ApplyDaze),
		(1, ApplyWeakness),
		(5, ApplyParalyzed),
		(10, KillWithXP)
	]


def OnSpellEffect (spell):
	npc = spell.caster
	caster_level = spell.caster_level
	if npc.name == 14286 or npc.name == 14358: # Balors
		caster_level = 20
		spell.dc = 25 #only affects banishment anyway

	tgts = CalculateTargets(ALIGNMENT_EVIL, caster_level, spell.target_list)
	(remove_list, unevil_list) = tgts

	parts = 'sp-Slay Living'
	WordEffect(spell, caster_level, effects, unevil_list, parts, remove_list)
def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

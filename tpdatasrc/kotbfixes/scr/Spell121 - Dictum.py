from toee import *
from alignment_words import *

def OnBeginSpellCast(spell): return

def ApplyDeafness(spell, target):
	dur = dice_new('1d4').roll()
	target.condition_add_with_args('Deafness', dur)

def ApplySlow(spell, target):
	dur = dice_new('2d4').roll()
	target.condition_add_with_args('Slow', dur, 0, spell.dc)

def ApplyParalyzed(spell, target):
	dur = 10 * dice_new('1d10').roll()
	target.condition_add_with_args('Paralyzed', dur, spell.dc)

effects = [
		(0, ApplyDeafness),
		(1, ApplySlow),
		(5, ApplyParalyzed),
		(10, KillWithXP)
	]


def OnSpellEffect(spell):
	caster_level = spell.caster_level
	tgts = CalculateTargets(ALIGNMENT_LAWFUL, caster_level, spell.target_list)
	(remove_list, unlawful_list) = tgts

	parts = 'sp-Destroy Undead'
	WordEffect(spell, caster_level, effects, unlawful_list, parts, remove_list)

def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

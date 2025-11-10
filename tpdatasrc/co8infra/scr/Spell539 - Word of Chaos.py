from toee import *
from alignment_words import *

def OnBeginSpellCast(spell): return

def ApplyDeafness(spell, target):
	dur = dice_new('1d4').roll()
	target.condition_add_with_args('Deafness', dur)

def ApplyStun(spell, target):
	target.condition_add_with_args('Stunned', 1)

def ApplyConfused(spell, target):
	dur = 10 * dice.new('1d10').roll()
	target.condition_add_with_args('Confused', dur)

effects = [
		(0, ApplyDeafness),
		(1, ApplyStun),
		(5, ApplyConfused),
		(10, KillWithXP)
	]


def OnSpellEffect(spell):
	caster_level = spell.caster_level
	tgts = CalculateTargets(ALIGNMENT_CHAOTIC, caster_level, spell.target_list)
	(remove_list, unchaos_list) = tgts

	parts = 'sp-Polymorph Other'
	WordEffect(spell, caster_level, effects, unchaos_list, parts, remove_list)

def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

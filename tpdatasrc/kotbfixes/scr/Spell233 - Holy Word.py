from toee import *
from alignment_words import *

def OnBeginSpellCast(spell):
	return

def ApplyDeafness(spell, target):
	dur = dice_new('1d4').roll()
	target.condition_add_with_args('Deafness', dur)

def ApplyBlindness(spell, target):
	dur = dice_new('2d4').roll()
	target.condition_add_with_args('Blindness', dur)

def ApplyParalyzed(spell, target):
	dur = 10 * dice_new('1d10').roll()
	# store DC for Remove Paralysis
	target.condition_add_with_args('Paralyzed', dur, spell.dc)

effects = [
		(0, ApplyDeafness),
		(1, ApplyBlindness),
		(5, ApplyParalyzed),
		(10, KillWithXP)
	]



def OnSpellEffect(spell):
	caster_level = spell.caster_level
	tgts = CalculateTargets(ALIGNMENT_GOOD, caster_level, spell.target_list)
	(remove_list, unevil_list) = tgts

	parts = 'sp-Holy Smite'
	WordEffect(spell, caster_level, effects, unevil_list, parts, remove_list)

def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

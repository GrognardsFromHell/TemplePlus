from toee import *
import tpdp

def OnBeginSpellCast( spell ):
	game.particles("sp-conjuration-conjure", spell.caster)


def	OnSpellEffect( spell ):
	remove_list = []

	spell.duration = 0

	bonus = 0
	# Remove Paralysis is less reliable if used on multiple targets
	if tpdp.config_get_bool("StricterRulesEnforcement"):
		if spell.num_of_targets == 2: bonus = 4
		elif spell.num_of_targets > 2: bonus = 2

	for target_item in spell.target_list:
		target = target_item.obj

		game.particles('sp-Remove Paralysis', target)

		target_item.partsys_id = game.particles('sp-Remove Paralysis', target)
		# duration is meaningless, so pack bonus in the second argument so it
		# can be inspected by paralysis conditions.
		target.condition_add_with_args('sp-Remove Paralysis', spell.id, bonus, 0)

		remove_list.append(target)

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

def OnBeginRound( spell ):
	print "Remove Paralysis OnBeginRound"

def OnEndSpellCast( spell ):
	print "Remove Paralysis OnEndSpellCast"

from toee import *
from banish_utils import IsOnHomePlane, IsBanishable

# Common utilities for spells like Holy Word and Blasphemy

# Checks if a creature can hear the spell. In 3.5, _some_ spells (e.g. Holy
# Word) mention that many of the effects only occur to creatures that hear
# the spell, while others do not. In 3.0, they all mentioned this, so I'm
# considering it to be an editing error in 3.5. The ones that don't actually
# still contain verbiage that _implies_ hearing is important.
#
# The bottom line is that this provides an important defense from these very
# powerful spells. The defense is not without trade-offs, either, so this
# just seems like proper game design.
def CannotHear(critter):
	in_silence = critter.d20_query_has_condition('sp-Silence Hit')
	deaf = critter.d20_query(Q_Critter_Is_Deafened)
	return in_silence or deaf

# Processes the target list, calculating potentially affected targets, and
# the `remove_list`.
#
# Copying items out of the `target_list` ensures that proper targeting
# happens. In the original Co8 scripts, some of the applied effects would
# cause mutation of `target_list` which can result in improper targeting of
# the actual effects.
def CalculateTargets(safe_align, cast_level, target_list):
	remove_list = []
	affected_list = []

	for target_item in target_list:
		target = target_item.obj

		# All targets will be removed at the end because these spells are
		# instantaneous duration.
		remove_list.append(target)

		if target.hit_dice_num > cast_level: continue

		if not (target.critter_get_alignment() & safe_align):
			affected_list.append(target)

	return (remove_list, affected_list)

def WordEffect(spell, cast_level, effects, targets, partname, remove_list):
	# If the caster is in an area of silence, the holy word is suppressed
	if spell.caster.d20_query_has_condition('sp-Silence Hit'):
		game.particles('Fizzle', spell.caster)

		spell.target_list.remove_list(remove_list)
		spell.spell_end(spell.id)
		return

	# banishment can only happen if the caster is in their home plane
	caster_in_home = IsOnHomePlane(spell.caster)

	for target in targets:
		if caster_in_home and IsBanishable(target):
			save = target.saving_throw_spell(
				spell.dc + 4, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id)
			if save:
				# saving throw successful
				target.float_mesfile_line('mes\\spell.mes', 30001)
			else:
				# saving throw unsuccessful
				target.float_mesfile_line('mes\\spell.mes', 30002)

				game.particles(partname, target)

				target.obj_set_obj(obj_f_last_hit_by, spell.caster)
				target.critter_banish(spell.caster)

				# target is gone, no need for other conditions
				continue

		# the remaining effects are contingent on the target hearing
		if CannotHear(target): continue

		game.particles(partname, target)

		for (threshold, apply) in effects:
			if target.hit_dice_num <= cast_level - threshold:
				apply(spell, target)

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

# Kills `target` while awarding `caster` the experience
def KillWithXP(spell, target):
	target.obj_set_obj(obj_f_last_hit_by, spell.caster)
	target.critter_kill()

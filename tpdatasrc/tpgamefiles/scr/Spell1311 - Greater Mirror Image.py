from toee import *

def OnBeginSpellCast(spell):
	game.particles('sp-illusion-conjure', spell.caster)

def OnSpellEffect(spell):
	clvl = spell.caster_level
	spell.duration = 10 * clvl

	# compute cap with a dice roll to incorporate empower
	cap = spell.roll_dice_with_metamagic(0,0,8)

	for target_item in spell.target_list:
		target = target_item.obj
		images = min(cap, spell.roll_dice_with_metamagic(1,4,clvl/3))

		target.condition_add_with_args(
				'sp-Greater Mirror Image', spell.id, spell.duration, images, 1, cap)
		game.particles('sp-Mirror Image', target)

	spell.caster.condition_add_with_args('Dismiss', spell.id)

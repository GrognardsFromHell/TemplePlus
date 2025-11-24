from toee import *
from tptrace import trace

def OnBeginSpellCast(spell):
	trace("Color Spray OnBeginSpellCast")
	trace("spell.target_list=", spell.target_list)
	trace("spell.caster=", spell.caster, " caster.level=", spell.caster_level)

def OnSpellEffect( spell ):
	trace("Color Spray OnSpellEffect")

	remove_list = []

	spell.duration = 0

	dc = spell.dc
	caster = spell.caster

	game.particles('sp-Color Spray', caster)

	for target_item in spell.target_list:
		target = target_item.obj
		if target.d20_query(Q_Critter_Is_Blinded):
			game.particles('Fizzle', target)
			remove_list.append(target)
			continue

		if target.saving_throw_spell(dc, D20_Save_Will, D20STD_F_NONE, caster, spell.id):
			# saving throw successful
			target.float_mesfile_line('mes\\spell.mes', 30001)
			game.particles('Fizzle', target)
			remove_list.append(target)
			continue

		# saving throw unsuccessful
		target.float_mesfile_line('mes\\spell.mes', 30002)

		if target.hit_dice_num >= 5:
			target.condition_add('sp-Color Spray Stun', spell.id)

		elif target.hit_dice_num >= 3:
			target.condition_add('sp-Color Spray Blind', spell.id)

		else:
			target.condition_add('sp-Color Spray Unconscious', spell.id)

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

def OnBeginRound(spell):
	trace("Color Spray OnBeginRound")

def OnEndSpellCast(spell):
	trace("Color Spray OnEndSpellCast")

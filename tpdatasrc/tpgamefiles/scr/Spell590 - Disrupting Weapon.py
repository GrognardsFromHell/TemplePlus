from toee import *

def OnBeginSpellCast(spell):
	game.particles('sp-transmutation-conjure', spell.caster)

def OnSpellEffect(spell):
	spell.duration = spell.caster_level

	remove = []
	for item in spell.target_list:
		target = item.obj

		valid = target.type == obj_t_weapon
		if valid:
			flags = target.obj_get_int(obj_f_weapon_flags)
			valid = (flags & OWF_RANGED_WEAPON) == 0

		if not valid:
			remove.append(target)
			game.sound(7461,1)
			game.particles('Fizzle', spell.caster)
			spell.caster.float_mesfile_line('mes\\spell.mes', 30003)
			continue

		# seems necessary for the condition to be added
		target.d20_status_init()
		target.condition_add_with_args(
				'sp-Disrupting Weapon', spell.id, spell.duration, spell.dc, 0, 0)

	spell.target_list.remove_list(remove)
	spell.spell_end(spell.id)

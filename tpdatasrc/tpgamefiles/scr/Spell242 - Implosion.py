from toee import *
from tpdp import SpellPacket

def OnBeginSpellCast(spell):
	game.particles('sp-evocation-conjure', spell.caster)

def OnBeginRound(spell):
	print "Implosion OnBeginRound"
	return

def dropItems(target):
	if target.type != obj_t_npc and target.type != obj_t_pc:
		return 0

	s

def OnSpellEffect(spell):
	spell.duration = 3

	packet = SpellPacket(spell.id)
	# concentration condition
	spell.caster.condition_add_with_args(
			'sp-Implosion', spell.id, spell.duration, spell.dc, 1, 0)

	target_item = spell.target_list[0]
	target = target_item.obj

	if packet.check_spell_resistance_force(target):
		game.particles('Fizzle', target)
		game.sound(17122,1)
		return

	if target.stat_level_get(stat_size) > STAT_SIZE_MEDIUM:
		game.particles('sp-Implosion-large', target)
	else:
		game.particles('sp-Implosion', target)

	saved = target.saving_throw_spell(
			spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id)
	if saved:
		game.sound(10849)
		return

	game.sound(10847)

	if target.type == obj_t_npc:
		target.obj_set_obj(obj_f_last_hit_by, spell.caster)
	spell.target_list.remove_target(target)
	target.critter_kill()
	target.object_flag_set(OF_DESTROYED)

def OnEndSpellCast(spell):
	print "Implosion end: " + str(spell.id)

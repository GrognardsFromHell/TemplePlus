from toee import *

debug = True

def Debug(*args):
	if debug:
		print "Enervation:",
		for arg in args:
			print arg,
		print ""

def OnBeginSpellCast(spell):
	Debug("OnBeginSpellCast")
	Debug("spell.target_list =", spell.target_list)
	Debug("spell.caster =", spell.caster, "caster.level =", spell.caster_level)

	game.particles("sp-necromancy-conjure", spell.caster)

def OnBeginProjectile(spell, projectile, index_of_target):
	Debug("OnBeginProjectile")

	part = game.particles('sp-Enervation-proj', projectile)
	projectile.obj_set_int(obj_f_projectile_part_sys_id, part)

def OnEndProjectile(spell, projectile, index_of_target):
	Debug("OnEndProjectile")

	dam_amount = spell.roll_dice_with_metamagic(1,4,0)

	Debug("drain roll = ", dam_amount)

	spell.duration = 0

	game.particles_end(projectile.obj_get_int(obj_f_projectile_part_sys_id))
	target_item = spell.target_list[0]
	target = target_item.obj

	attack_roll = spell.caster.perform_touch_attack(target)
	if attack_roll & D20CAF_HIT:
		if (target.is_category_type(mc_type_undead)):
			temp_hp = dam_amount * 5
			target.condition_add_with_args('sp-False Life', spell.id, 600, temp_hp)
			target_item.partsys_id = game.particles('sp-False Life', target)
		else:
			if attack_roll & D20CAF_CRITICAL:
				target.float_mesfile_line('mes\\combat.mes', 12, tf_red)
				dam_amount = dam_amount * 2

			target.float_mesfile_line('mes\\combat.mes', 6016, tf_red)
			target_item.partsys_id = game.particles('sp-Enervation-hit', target)

			# set attribute for proper XP award in case of death
			if target.type == obj_t_npc:
				target.obj_set_obj(obj_f_last_hit_by, spell.caster)

			for i in range(dam_amount):
				# Args are:
				#	class affected
				#	DC for becoming permanent
				#	inventory slot causing (irrelevant)
				#
				# Set DC = -1 for Enervation so that these
				# levels should never become permanent
				target.condition_add_with_args('Temp Negative Level', 0, -1, 0)

	else:
		# missed
		target.float_mesfile_line('mes\\spell.mes', 30007)

		game.particles('Fizzle', target)

	spell.target_list.remove_target(target)

	spell.spell_end(spell.id)

def OnEndSpellCast(spell):
	Debug("OnEndSpellCast")

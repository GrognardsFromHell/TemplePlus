from toee import *

air_node = 5081
earth_node = 5082
fire_node = 5083
water_node = 5084

nodes = set([air_node, earth_node, fire_node, water_node])

def IsOnHomePlane(critter):
	# nodes are created pocked planes with no natives
	if critter.map in nodes: return False

	# all other maps are (I think) prime material plane, so extraplanar flags
	# apply, either monster subtype or npc flags.
	is_extra_sub = critter.is_category_subtype(mc_subtype_extraplanar)
	is_extra_flag = critter.npc_flags_get() & ONF_EXTRAPLANAR
	return not (is_extra_sub or is_extra_flag)

# checks if a creature can hear the holy word, because most effects occur only
# to people that hear the word
def CannotHear(critter):
	in_silence = critter.d20_query_has_condition('sp-Silence Hit')
	deaf = critter.d20_query(Q_Critter_Is_Deafened)
	return in_silence or deaf

def OnBeginSpellCast(spell):
	return

def OnSpellEffect(spell):
	remove_list = []
	ungood_list = []

	# Copy valid targets to a separate list. Apparently prevents some oddities
	# with the wrong targets being hit when the target list is used directly.
	#
	# http://www.co8.org/community/index.php?threads/holy-word-killed-my-cg-pc.12164/#post-145537
	for target_item in spell.target_list:
		target = target_item.obj

		alignment = target.critter_get_alignment()

		if not (alignment & ALIGNMENT_GOOD):
			ungood_list.append(target)

		# all targets will be removed at the end because Holy Word is an
		# instantaneous spell
		remove_list.append(target)

	# If the caster is in an area of silence, the holy word is suppressed
	if spell.caster.d20_query_has_condition('sp-Silence Hit'):
		game.particles('Fizzle', spell.caster)

		spell.target_list.remove_list(remove_list)
		spell.spell_end(spell.id)
		return

	# banishment effect only occurs on the caster's home plane
	caster_in_home = IsOnHomePlane(spell.caster)

	for target in ungood_list:
		if caster_in_home:
			summoned = target.d20_query_has_spell_condition(sp_Summoned)
			can_banish = summoned or not IsOnHomePlane(target)
			if can_banish:
				save = target.saving_throw_spell(
					spell.dc + 4, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id)
				if save:
					target.float_mesfile_line('mes\\spell.mes', 30001)
				else:
					# saving throw unsuccessful
					target.float_mesfile_line('mes\\spell.mes', 30002)

					game.particles('sp-Holy Smite', target)

					target.obj_set_obj(obj_f_last_hit_by, spell.caster)
					target.critter_banish(spell.caster)
					# target is gone, no need for other conditions
					continue

		# The remaining effects are contingent on the target hearing the holy word.
		if CannotHear(target): continue

		game.particles('sp-Holy Smite', target)

		# use non-spell conditions, because Holy Word is not an ongoing spell, it
		# is an instantaneous spell that inflicts ongoing conditions.
		if target.hit_dice_num <= spell.caster_level:
			dur = dice_new('1d4').roll()
			target.condition_add_with_args('Deafness', dur)

		if target.hit_dice_num <= spell.caster_level-1:
			dur = dice_new('2d4').roll()
			target.condition_add_with_args('Blindness', dur)

		if target.hit_dice_num <= spell.caster_level-5:
			dur = 10 * dice_new('1d10').roll()
			# store DC for Remove Paralysis
			target.condition_add_with_args('Paralyzed', dur, spell.dc)

		if target.hit_dice_num <= spell.caster_level-10:
			target.obj_set_obj(obj_f_last_hit_by, spell.caster)
			target.critter_kill()

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

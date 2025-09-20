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

# Checks if a creature can hear the blasphemy, because most effects occur only
# to people that hear. Strictly speaking, this is not in the SRD text.
# However, other versions of this spell (e.g. Holy Word) _do_ say it, _and_
# the 3.0 version of Blasphemy said it. So I consider it an editing error
# that 3.5 Blasphemy doesn't.
def CannotHear(critter):
	in_silence = critter.d20_query_has_condition('sp-Silence Hit')
	deaf = critter.d20_query(Q_Critter_Is_Deafened)
	return in_silence or deaf

def OnBeginSpellCast(spell):
	return

def OnSpellEffect (spell):
	remove_list = []
	unevil_list = []

	npc = spell.caster
	spell_caster_level = spell.caster_level
	if npc.name == 14286 or npc.name == 14358: # Balors
		spell_caster_level = 20
		spell.dc = 25 #only affects banishment anyway

	# Copy valid targets to a separate list to avoid mutation of target_list
	# from killed critters or such.
	for target_item in spell.target_list:
		target = target_item.obj

		# all targets will be removed at the end because Blasphemy is an
		# instantaneous spell
		remove_list.append(target)

		# can't affect creatures with more hit dice than the caster level
		if target.hit_dice_num > spell_caster_level: continue

		alignment = target.critter_get_alignment()

		if not (alignment & ALIGNMENT_EVIL):
			unevil_list.append(target)

	# If the caster is in an area or silence, the blasphemy is suppressed
	if spell.caster.d20_query_has_condition('sp-Silence Hit'):
		game.particles('Fizzle', spell.caster)

		spell.target_list.remove_list(remove_list)
		spell.spell_end(spell.id)
		return

	caster_in_home = IsOnHomePlane(spell.caster)

	for target in unevil_list:
		if caster_in_home:
			summoned = target.d20_query_has_spell_condition(sp_Summoned)
			can_banish = summoned or not IsOnHomePlane(target)
			if can_banish:
				save = target.saving_throw_spell(
					spell.dc + 4, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id)
				if save:
					# saving throw successful
					target.float_mesfile_line('mes\\spell.mes', 30001)
				else:
					# saving throw unsuccessful
					target.float_mesfile_line('mes\\spell.mes', 30002)

					game.particles('sp-Slay Living', target)

					target.obj_set_obj(obj_f_last_hit_by, spell.caster)
					target.critter_banish(spell.caster)

					# target is gone, no need for other conditions
					continue


		# the remaining effects are contingent on the target hearing the
		# blasphemy
		if CannotHear(target): continue

		game.particles('sp-Slay Living', target)

		# avoid attaching conditions to spells, because Blasphemy is an
		# instantaneous spell that causes ongoing effects, not an ongoing spell
		# with effects contingent on the spell being active.

		# caster level or less is dazed for 1 round
		target.condition_add_with_args('sp-Daze', 0, 1, 0)

		# below caster level is weakened for 2d4 rounds
		if target.hit_dice_num <= spell_caster_level-1:
			dur = dice_new('2d4').roll()
			penalty = dice_new('2d6').roll()
			target.condition_add_with_args('Weakness', dur, 0, penalty)

		# caster level-5 or less is paralyzed
		if target.hit_dice_num <= spell_caster_level-5:
			dur = 10 * dice_new('1d10').roll()
			target.condition_add_with_args('Paralyzed', dur, spell.dc)

		# caster level-10 or less is killed
		if target.hit_dice_num <= spell_caster_level-10:
			target.obj_set_obj(obj_f_last_hit_by, spell.caster)
			target.critter_kill()

	spell.target_list.remove_list(remove_list)
	spell.spell_end(spell.id)

	

def OnBeginRound(spell): return

def OnEndSpellCast(spell): return

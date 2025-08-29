from toee import *
from utilities import *
import tpdp

def OnBeginSpellCast( spell ):
	print "Silence OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-illusion-conjure", spell.caster )

def	OnSpellEffect( spell ):
	print "Silence OnSpellEffect"

	npc = spell.caster			##  added so NPC's can use wand/potion/scroll
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL and spell.caster_level <= 0:
		spell.caster_level = 8

	if npc.name == 14425 and game.global_vars[711] == 1:
		spell.caster_level = 6
		spell.dc = 17

	spell.duration = 10 * spell.caster_level

	part_name = 'sp-Silence'
	if spell.spell_radius > 20:
		part_name = part_name + '-WIDE'

	caster = spell.caster
	packet = tpdp.SpellPacket(spell.id)

	# test whether we targeted the ground or an object
	if spell.is_object_selected() == 1:
		target_item = spell.target_list[0]
		target = target_item.obj

		avoid = False
		# If the target is friendly, assume they're willing, since putting
		# mobile silence on a friendly fighter is a valid tactic.
		if target != caster and not target.is_friendly(caster):
			# check spell resistance; need to mark the spell as 'SR: no' to avoid
			# incorrect AoE behavior under strict rules, so there's no automatic
			# check.
			avoid = packet.check_spell_resistance_force(target)
			if not avoid:
				# allow Will saving throw to negate
				avoid = target.saving_throw_spell(
						spell.dc, D20_Save_Will, D20STD_F_NONE, caster, spell.id)

				if avoid:
					# saving throw successful
					target_item.obj.float_mesfile_line('mes\\spell.mes', 30001)

					game.particles('Fizzle', target)
				else:
					# saving throw failed
					target.float_mesfile_line('mes\\spell.mes', 30002)

		if avoid:
			spell.spell_end(spell.id, 1) # force end
		else:
			spell_obj_partsys_id = game.particles(part_name, target)
			target.condition_add_with_args(
					'sp-Silence', spell.id, spell.duration, 0, spell_obj_partsys_id)

	else:
		# spawn one spell_object object

		if npc.name == 14425 and npc.map == 5065 and game.global_vars[711] == 1:
			game.global_vars[711] = 2
			spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, 2126008812006L )
		else:
			spell_obj = game.obj_create( OBJECT_SPELL_GENERIC, spell.target_loc, spell.target_loc_off_x, spell.target_loc_off_y )

		# add to d20initiative
		caster_init_value = spell.caster.get_initiative()
		spell_obj.d20_status_init()
		spell_obj.set_initiative( caster_init_value )

		# put sp-Silence condition on obj
		spell_obj_partsys_id = game.particles(part_name, spell_obj )
		spell_obj.condition_add_with_args(
				'sp-Silence', spell.id, spell.duration, 0, spell_obj_partsys_id )

def OnBeginRound( spell ):
	print "Silence OnBeginRound"

def OnEndSpellCast( spell ):
	print "Silence OnEndSpellCast"

def OnAreaOfEffectHit( spell ):
	print "Silence OnAreaOfEffectHit"

def OnSpellStruck( spell ):
	print "Silence OnSpellStruck"

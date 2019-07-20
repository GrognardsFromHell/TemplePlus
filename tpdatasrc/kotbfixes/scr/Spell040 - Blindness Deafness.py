from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Blindness/Deafness OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )
	return 1

def OnSpellEffect( spell ):
	print "Blindness/Deafness OnSpellEffect"

	spell.duration = 14400
	target_item = spell.target_list[0]

	## Solves Radial menu problem for Wands/NPCs
	spell_arg = spell.spell_get_menu_arg( RADIAL_MENU_PARAM_MIN_SETTING )
	if spell_arg != 1 and spell_arg != 2:
		spell_arg = game.random_range(1,2)

	npc = spell.caster
	if npc.name == 14609:			##  special for Zuggtmoy Priest
		spell_arg = 2

	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		if critter_is_unconscious(target_item.obj) != 1 and not target_item.obj.d20_query(Q_Prone) and (target_item.obj.stat_level_get(stat_level_wizard) >= 3 or target_item.obj.stat_level_get(stat_level_sorcerer) >= 3 or target_item.obj.stat_level_get(stat_level_bard) >= 3):
			npc = spell.caster
		else:
			for obj in game.party[0].group_list():
				if critter_is_unconscious(obj) != 1 and not obj.d20_query(Q_Prone) and (obj.stat_level_get(stat_level_wizard) >= 3 or obj.stat_level_get(stat_level_sorcerer) >= 3 or obj.stat_level_get(stat_level_bard) >= 3):
					target_item.obj = obj

	# allow Fortitude saving throw to negate
	if target_item.obj.saving_throw_spell( spell.dc, D20_Save_Fortitude, D20STD_F_NONE, spell.caster, spell.id ):
		# saving throw successful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
		game.particles( 'Fizzle', target_item.obj )
		spell.target_list.remove_target( target_item.obj )

	else:
		# saving throw unsuccessful
		target_item.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

		if spell_arg == 1:
			# apply blindness
			return_val = target_item.obj.condition_add_with_args( 'sp-Blindness', spell.id, spell.duration, 0 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Blindness-Deafness', target_item.obj )

		else:
			# apply deafness
			return_val = target_item.obj.condition_add_with_args( 'sp-Deafness', spell.id, spell.duration, 0 )
			if return_val == 1:
				target_item.partsys_id = game.particles( 'sp-Blindness-Deafness', target_item.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Blindness/Deafness OnBeginRound"

def OnEndSpellCast( spell ):
	print "Blindness/Deafness OnEndSpellCast"
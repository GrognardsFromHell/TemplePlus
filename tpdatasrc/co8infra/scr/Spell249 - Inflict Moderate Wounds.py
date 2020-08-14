from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Inflict Moderate Wounds OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Inflict Moderate Wounds OnSpellEffect"

	dice = dice_new( "2d8" )
	dice.bonus = min( 10, spell.caster.stat_level_get( spell.caster_class ) )

	target = spell.target_list[0]

	npc = spell.caster			##  added so NPC's will choose valid targets
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		if critter_is_unconscious(target.obj) != 1 and not target.obj.d20_query(Q_Prone):
			npc = spell.caster
		else:
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 10 and critter_is_unconscious(obj) != 1 and not obj.d20_query(Q_Prone):
					target.obj = obj

	# check if target is friendly (willing target)
	if target.obj.is_friendly( spell.caster ):

		# check if target is undead
		if target.obj.is_category_type( mc_type_undead ):
			target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
		else:
			# damage target
			if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

				# saving throw succesful, damage target, 1/2 damage
				target.obj.spell_damage_with_reduction( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id )
			else:
				target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

				# saving throw unsuccesful, damage target, full damage
				target.obj.spell_damage( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spell.id )

	else:
		attack_result = spell.caster.perform_touch_attack(target.obj, 1)
		if attack_result & D20CAF_HIT:
			# check if target is undead
			if target.obj.is_category_type( mc_type_undead ):
				# check saving throw, heal target
				if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
					#target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

					# saving throw succesful, heal target, 1/2 heal
					target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
				else:
					#target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# saving throw unsuccesful, heal target, full heal
					target.obj.spell_heal( spell.caster, dice, D20A_CAST_SPELL, spell.id )
			else:
				# check saving throw, damage target
				if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
					target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )

					# saving throw succesful, damage target, 1/2 damage
					target.obj.spell_damage_weaponlike( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, DAMAGE_REDUCTION_HALF, D20A_CAST_SPELL, spell.id, attack_result, 0 )
				else:
					target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )

					# saving throw unsuccesful, damage target, full damage
					target.obj.spell_damage_weaponlike( spell.caster, D20DT_NEGATIVE_ENERGY, dice, D20DAP_UNSPECIFIED, 100, D20A_CAST_SPELL, spell.id, attack_result, 0 )

	game.particles( 'sp-Inflict Moderate Wounds', target.obj )

	spell.target_list.remove_target( target.obj )
	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Inflict Moderate Wounds OnBeginRound"

def OnEndSpellCast( spell ):
	print "Inflict Moderate Wounds OnEndSpellCast"
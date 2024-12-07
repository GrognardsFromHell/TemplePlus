from toee import *
from utilities import *

def OnBeginSpellCast( spell ):
	print "Doom OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-necromancy-conjure", spell.caster )

def OnSpellEffect( spell ):
	print "Doom OnSpellEffect"

	spell.duration = 10 * spell.caster_level

	target = spell.target_list[0]

	npc = spell.caster			##  added so NPC's will choose valid targets
	if npc.type != obj_t_pc and npc.leader_get() == OBJ_HANDLE_NULL:
		if (target.obj.type == obj_t_pc or target.obj.type == obj_t_npc) and critter_is_unconscious(target.obj) != 1 and not target.obj.d20_query(Q_Prone):
			npc = spell.caster

		else:
			game.global_flags[811] = 0	
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 5 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 10 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 15 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 20 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 25 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 30 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1
			for obj in game.party[0].group_list():
				if obj.distance_to(npc) <= 100 and critter_is_unconscious(obj) != 1 and game.global_flags[811] == 0 and not obj.d20_query(Q_Prone) and (obj.type == obj_t_pc or obj.type == obj_t_npc):
					target.obj = obj
					game.global_flags[811] = 1

	if (target.obj.type == obj_t_pc) or (target.obj.type == obj_t_npc):
		# allow Will saving throw to negate
		if target.obj.saving_throw_spell( spell.dc, D20_Save_Will, D20STD_F_NONE, spell.caster, spell.id ):
			# saving throw successful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30001 )
			game.particles( 'Fizzle', target.obj )
			spell.target_list.remove_target( target.obj )

		else:
			# saving throw unsuccessful
			target.obj.float_mesfile_line( 'mes\\spell.mes', 30002 )
			# HTN - apply condition DOOM
			target.obj.condition_add_with_args( 'sp-Doom', spell.id, spell.duration, 0 )
			target.partsys_id = game.particles( 'sp-Doom', target.obj )

	spell.spell_end( spell.id )

def OnBeginRound( spell ):
	print "Doom OnBeginRound"

def OnEndSpellCast( spell ):
	print "Doom OnEndSpellCast"
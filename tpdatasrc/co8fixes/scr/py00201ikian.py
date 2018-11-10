from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog(attachee,1)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				obj.begin_dialog(attachee,1)
				game.new_sid = 0
				return RUN_DEFAULT
	return RUN_DEFAULT


def all_run_off( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
		if (obj.leader_get() == OBJ_HANDLE_NULL and not obj in game.leader.group_list()):
			obj.runoff(obj.location-3)
	return RUN_DEFAULT


def buff_npc( attachee, triggerer ):
	game.global_vars[761] = game.global_vars[761] + 1
	if (game.global_vars[761] == 1):
		attachee.cast_spell(spell_shield_of_faith, attachee)
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14333 and obj.leader_get() == OBJ_HANDLE_NULL):
				obj.cast_spell(spell_mage_armor, obj)
			if (obj.name == 14336 and obj.leader_get() == OBJ_HANDLE_NULL):
				obj.cast_spell(spell_resist_elements, obj)
	if (game.global_vars[761] == 2):
		attachee.cast_spell(spell_owls_wisdom, attachee)
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14333 and obj.leader_get() == OBJ_HANDLE_NULL):
				obj.cast_spell(spell_mirror_image, obj)
	if (game.global_vars[761] >= 3):
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14333 and obj.leader_get() == OBJ_HANDLE_NULL):
				obj.cast_spell(spell_shield, obj)
		for obj in game.obj_list_vicinity(attachee.location,OLC_NPC):
			if (obj.name == 14334 and obj.leader_get() == OBJ_HANDLE_NULL):
				attachee.cast_spell(spell_endure_elements, obj)
	return RUN_DEFAULT


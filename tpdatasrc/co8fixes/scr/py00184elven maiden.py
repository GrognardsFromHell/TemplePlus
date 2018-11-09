from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (not game.combat_is_active()):
		near_pc = OBJ_HANDLE_NULL
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (is_safe_to_talk(attachee,obj)):
				near_pc = obj
				if ((obj.stat_level_get(stat_alignment) == LAWFUL_GOOD) or (obj.stat_level_get(stat_alignment) == NEUTRAL_GOOD) or (obj.stat_level_get(stat_alignment) == CHAOTIC_GOOD)):
					obj.begin_dialog(attachee,1)
					game.new_sid = 0
					return RUN_DEFAULT
		if (near_pc != OBJ_HANDLE_NULL):
			near_pc.begin_dialog(attachee,1)
			game.new_sid = 0
	return RUN_DEFAULT


def money_handout(npc,pc):
	for obj in pc.group_list():
		obj.money_adj(100000)
	return RUN_DEFAULT


def all_run_off(npc, pc):
	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.leader_get() == OBJ_HANDLE_NULL and not obj in game.leader.group_list()):
			obj.runoff(obj.location-3)
	return

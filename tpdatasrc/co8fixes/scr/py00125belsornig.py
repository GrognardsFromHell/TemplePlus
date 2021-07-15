from toee import *
from py00439script_daemon import record_time_stamp, get_v, set_v, npc_set, npc_unset, npc_get, tsc, tpsts, within_rect_by_corners
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	record_time_stamp(515)
	if (game.global_flags[132] == 1):
		attachee.attack(triggerer)
	elif (not attachee.has_met( triggerer )):
		triggerer.begin_dialog( attachee, 1 )
	else:
		triggerer.begin_dialog( attachee, 370 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[372] == 1):
		attachee.object_flag_set(OF_OFF)
	else:
		if (attachee.leader_get() == OBJ_HANDLE_NULL):
			game.global_vars[714] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[105] = 1
	record_time_stamp(457)
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	game.global_flags[345] = 0
	if (not attachee.has_wielded(4071) or not attachee.has_wielded(4124)):
		attachee.item_wield_best_all()
	for statue in game.obj_list_vicinity( attachee.location, OLC_SCENERY ):
		if statue.object_flags_get() & OF_DONTDRAW: # fixes issue in reactive temple where the old state would come alive instead of the relocated one
			continue
		if (statue.name == 1618):
			loc = statue.location
			rot = statue.rotation
			statue.destroy()
			for obj in game.obj_list_vicinity( attachee.location, OLC_NPC ):
				if obj.name == 14244:
					return RUN_DEFAULT					
			juggernaut = game.obj_create( 14244, loc )
			juggernaut.rotation = rot
			game.particles( "ef-MinoCloud", juggernaut )
			return RUN_DEFAULT
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[105] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[714] == 16 and not attachee.has_wielded(4071) or not attachee.has_wielded(4124)):
		attachee.item_wield_best_all()
		attachee.item_wield_best_all()
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (not attachee.has_met(obj)):
				if (is_safe_to_talk(attachee,obj)):
					record_time_stamp(515)
					if ( (game.global_flags[104] == 1) or (game.global_flags[106] == 1) or (game.global_flags[107] == 1) ):
						obj.turn_towards(attachee)	## added by Livonya
						attachee.turn_towards(obj)	## added by Livonya
						obj.begin_dialog( attachee, 600 )
					else:
						obj.turn_towards(attachee)	## added by Livonya
						attachee.turn_towards(obj)	## added by Livonya
						obj.begin_dialog(attachee,1)
#						game.new_sid = 0			## removed by Livonya
	if (game.global_vars[714] == 0 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		attachee.cast_spell(spell_protection_from_elements, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[714] == 4 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		attachee.cast_spell(spell_freedom_of_movement, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[714] == 8 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		attachee.cast_spell(spell_endurance, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[714] == 12 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
#		attachee.cast_spell(spell_fog_cloud, attachee)
		attachee.cast_spell(spell_shield_of_faith, attachee)
		attachee.spells_pending_to_memorized()
	game.global_vars[714] = game.global_vars[714] + 1

	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_flags[132] == 0):
		return SKIP_DEFAULT
	else:
		return RUN_DEFAULT
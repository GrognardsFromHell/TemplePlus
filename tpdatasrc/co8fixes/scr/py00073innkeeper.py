from utilities import *
from __main__ import game
from toee import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.map == 5008): #upstairs - arranging for lodging for a PC
		triggerer.begin_dialog( attachee, 420 )	
	if (attachee.map == 5006):
		triggerer.begin_dialog( attachee, 350 )	
	elif ((anyone( triggerer.group_list(), "has_follower", 8003 )) and ((game.quests[18].state == qs_unknown) or (game.quests[18].state == qs_mentioned) or (game.quests[18].state == qs_accepted))):
		triggerer.begin_dialog( attachee, 200 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map == 5007):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			attachee.object_flag_unset(OF_OFF)
	elif (attachee.map == 5006):
		if (game.global_vars[510] != 2):
			if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6):
				attachee.object_flag_unset(OF_OFF)
		else:
			attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def set_room_flag( attachee, triggerer ):
	game.global_flags[56] = 1
	game.timeevent_add( room_no_longer_available, (), 86390000 )
	game.sleep_status_update()
	return RUN_DEFAULT
	

def room_no_longer_available():
	game.global_flags[56] = 0
	game.sleep_status_update()
	return RUN_DEFAULT

def can_stay_behind(obj):
	if obj.type == obj_t_pc:
		return 1
	return 0

def mark_pc_dropoff(obj):
	obj.scripts[9] = 439
	set_f('pc_dropoff')
	
def contest_who( attachee ):
	for n in [8010, 8005, 8011, 8000]:
		npc = find_npc_near(attachee,n)
		if (npc != OBJ_HANDLE_NULL):
			npc.float_line( 300, attachee )		
	return RUN_DEFAULT


def contest_drink( attachee, triggerer ):
	npcs_awake = 0
	for n in [8009, 8010, 8005, 8011]:
		npc = find_npc_near(attachee,n)
		if (npc != OBJ_HANDLE_NULL):
			if (npc.stat_level_get(stat_subdual_damage) < npc.stat_level_get(stat_hp_current)):
				npc.float_line( 301, attachee)
				npc.damage(OBJ_HANDLE_NULL,D20DT_SUBDUAL,dice_new("1d3"),D20DAP_NORMAL)
				if (npc.stat_level_get(stat_subdual_damage) < npc.stat_level_get(stat_hp_current)):
					npcs_awake = npcs_awake + 1;
	damage_dice = dice_new("1d3")
	damage = damage_dice.roll()
	if ((triggerer.stat_level_get(stat_subdual_damage)+damage) >= triggerer.stat_level_get(stat_hp_current)):
		if (npcs_awake == 0):
			attachee.float_line( 310, triggerer)
		elif (npcs_awake == 1):
			attachee.float_line( 300, triggerer)
		else:
			attachee.float_line( 280, triggerer)
	else:
		damage_dice = dice_new("0d3")
		damage_dice.bonus = damage
		triggerer.damage(OBJ_HANDLE_NULL,D20DT_SUBDUAL,damage_dice,D20DAP_NORMAL)
		if (npcs_awake == 0):
			triggerer.begin_dialog( attachee, 290 )
		else:
			triggerer.begin_dialog( attachee, 270 )
	return RUN_DEFAULT
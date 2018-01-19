from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	triggerer.begin_dialog(attachee,1)
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
		attachee.turn_towards(obj)
		if (is_safe_to_talk(attachee,obj)):
			obj.begin_dialog(attachee,1)
			game.new_sid = 0
			return RUN_DEFAULT
	return RUN_DEFAULT

	

	
def loot_caravan_bandits( pc, charity = 0 ):
	pc_index = 0
	for obj in game.obj_list_vicinity(pc.location, OLC_NPC):
		if obj.name == 14317: # Caravan Bandits
			listt = [6042, 6043, 6044 ,6045, 6046, 4074, 4071, 4067, 4116, 4036, 4096, 6034]
			if charity == 0:
				listt.append(7001)
			for item_number in listt:
				countt = 0
				while obj.item_find(item_number) != OBJ_HANDLE_NULL and countt <= 20 and pc_index < len(game.party): ## count <= added as failsafe (in case PC is overloaded and something freaky happens...)
					tempp = game.party[pc_index].item_get(obj.item_find(item_number) )
					if tempp == 0:
						pc_index +=1
					countt += 1	
	game.fade(0,0,1009,0)
	start_game_with_quest(22)
	
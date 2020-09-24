from toee import *
from utilities import *
from combat_standard_routines import *


def san_use( attachee, triggerer ):
	if (attachee.name == 1034):
		create_item_in_inventory( 6311, chest )
		game.new_sid = 0	
		return RUN_DEFAULT
	npc = find_npc_near(attachee,8063)
	game.new_sid = 0
	if (npc != OBJ_HANDLE_NULL and not npc.is_unconscious() ):
		npc.turn_towards(triggerer)
		triggerer.begin_dialog( npc, 60 )
		return SKIP_DEFAULT	
#		for obj in game.obj_list_vicinity(npc.location,OLC_PC):
#			if (is_safe_to_talk(obj,npc)):
#				npc.turn_towards(obj)
#				obj.turn_towards(npc)
#				obj.begin_dialog( npc, 60 )
#				return SKIP_DEFAULT	
	return RUN_DEFAULT



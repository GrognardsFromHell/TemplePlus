from toee import *
from combat_standard_routines import *
from utilities import *


def san_insert_item( attachee, triggerer ):
	if (( triggerer.name == 8048 ) or ( triggerer.name == 8049 ) or ( triggerer.name == 1204 )):
		game.encounter_queue.append(3000)
		
	elif triggerer.name == 1004 and game.leader.map == 5010: #5010- Trader's shop
		if not 3000 in game.encounter_queue:
			game.encounter_queue.append(3000)
			
	elif triggerer.name == 1001 and game.leader.map == 5001 and attachee.name in [1, 3006, 4120, 6097, 6098, 6099, 6100]: # note: for some reason most of Lareth's item have a "name" field of 1, and another has 3006. Will be fixed in the future inside Protos.tab, for now this is a hotfix
		bro_smith = OBJ_HANDLE_NULL
		for npc in game.obj_list_vicinity( triggerer.location, OLC_NPC):
			if npc.name == 20005:
				bro_smith = npc
		if bro_smith == OBJ_HANDLE_NULL:
			return SKIP_DEFAULT
		if is_safe_to_talk(bro_smith, game.leader):
			pc_dude = game.leader
		else:
			pc_dude = party_closest(npc, 1, 0)
			
		if game.global_vars[452] & (2**6) == 0:
			if  pc_dude != OBJ_HANDLE_NULL and pc_dude.type == obj_t_pc:
				cur_money = pc_dude.money_get()
				game.char_ui_hide()
				game.timevent_add( smith_refund, ( cur_money ), 450, 1) # give money back to Smith
				game.timevent_add( pc_dude.item_get, ( attachee ), 750, 1) # get item back
				game.global_vars[452] |= 2**6
				pc_dude.begin_dialog( bro_smith, 1000 )
		elif attachee.obj_get_int(obj_f_item_wear_flags) & 256 != 0 and attachee.scripts[21] == 186: # Lareth's plate boots
			cur_money = pc_dude.money_get()
			game.char_ui_hide()
			game.timevent_add( smith_refund, ( cur_money ), 450, 1) # give money back to Smith
			game.timevent_add( pc_dude.item_get, ( attachee ), 750, 1) # get item back
			pc_dude.begin_dialog( bro_smith, 1050 )
			
	
	return SKIP_DEFAULT

def smith_refund(cur_money):
	game.leader.money_adj( cur_money - game.leader.money_get() )
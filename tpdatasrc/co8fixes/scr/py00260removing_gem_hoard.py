from toee import *
from py00176zuggtmoy import transform_into_demon_form
from combat_standard_routines import *


def san_remove_item( attachee, triggerer ):
	if game.party[0].map == 5079 and triggerer.name == 1050: ## Zuggtmoy level, taking from throne of gems
		zuggtmoy = find_npc_near(triggerer, 8064)
		loc = triggerer.location
		rot = triggerer.rotation
		triggerer.destroy()
		empty_throne = game.obj_create( 1051, loc )
		empty_throne.rotation = rot
		game.char_ui_hide()
		if ((zuggtmoy != OBJ_HANDLE_NULL) and (game.leader != OBJ_HANDLE_NULL)):
			if (game.global_flags[181] == 1):
				game.global_flags[181] = 0
				transform_into_demon_form(zuggtmoy,game.leader,320)
			else:
				game.leader.begin_dialog(zuggtmoy,320)
		game.new_sid = 0
		return RUN_DEFAULT
	else:
		return SKIP_DEFAULT
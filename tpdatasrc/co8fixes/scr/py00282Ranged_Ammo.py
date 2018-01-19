from toee import *
from utilities import *
from combat_standard_routines import *


def san_insert_item( attachee, triggerer ):
	done = attachee.obj_get_int( obj_f_weapon_pad_i_1 )
	if (triggerer.type == obj_t_pc or triggerer.type == obj_t_npc) and (triggerer.has_feat(feat_far_shot)):
		if done == 1:
			return RUN_DEFAULT
		else:
			curr = attachee.obj_get_int( obj_f_weapon_range )
			curr = curr * 1.5
			attachee.obj_set_int( obj_f_weapon_range, curr )
			attachee.obj_set_int( obj_f_weapon_pad_i_1, 1 )
			game.sound(3013,1)
	else:
		if done == 1:
			curr = attachee.obj_get_int( obj_f_weapon_range )
			curr = curr * 2/3
			attachee.obj_set_int( obj_f_weapon_range, curr )
			attachee.obj_set_int( obj_f_weapon_pad_i_1, 0 )
			game.sound(3013,1)
	return RUN_DEFAULT

## done = 0 when in the hands of someone without far_shot, 1 when in the hands of someone with the feat
from toee import *
from utilities import *
from combat_standard_routines import *


def san_enter_combat( attachee, triggerer ):
	# this is a problematic script, removing -SA
	# if (attachee.leader_get() == OBJ_HANDLE_NULL):
		# if (triggerer.type == obj_t_pc):
			# if anyone( triggerer.group_list(), "has_follower", 8000 ):
				# elmo = find_npc_near( triggerer, 8000 )
				# if (elmo != OBJ_HANDLE_NULL):
					# game.global_vars[900] = game.global_vars[900] + 1
					# elmo.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8001 ):
				# paida = find_npc_near( triggerer, 8001 )
				# if (paida != OBJ_HANDLE_NULL):
					# game.global_vars[902] = game.global_vars[902] + 1
					# paida.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8014 ):
				# otis = find_npc_near( triggerer, 8014 )
				# if (otis != OBJ_HANDLE_NULL):
					# game.global_vars[903] = game.global_vars[903] + 1
					# otis.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8015 ):
				# meleny = find_npc_near( triggerer, 8015 )
				# if (meleny != OBJ_HANDLE_NULL):
					# game.global_vars[904] = game.global_vars[904] + 1
					# meleny.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8021 ):
				# ydey = find_npc_near( triggerer, 8021 )
				# if (ydey != OBJ_HANDLE_NULL):
					# game.global_vars[905] = game.global_vars[905] + 1
					# ydey.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8022 ):
				# murfles = find_npc_near( triggerer, 8022 )
				# if (murfles != OBJ_HANDLE_NULL):
					# game.global_vars[906] = game.global_vars[906] + 1
					# murfles.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8031 ):
				# thrommel = find_npc_near( triggerer, 8031 )
				# if (thrommel != OBJ_HANDLE_NULL):
					# game.global_vars[907] = game.global_vars[907] + 1
					# thrommel.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8039 ):
				# taki = find_npc_near( triggerer, 8039 )
				# if (taki != OBJ_HANDLE_NULL):
					# game.global_vars[908] = game.global_vars[908] + 1
					# taki.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8054 ):
				# burne = find_npc_near( triggerer, 8054 )
				# if (burne != OBJ_HANDLE_NULL):
					# game.global_vars[909] = game.global_vars[909] + 1
					# burne.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8060 ):
				# morgan = find_npc_near( triggerer, 8060 )
				# if (morgan != OBJ_HANDLE_NULL):
					# game.global_vars[910] = game.global_vars[910] + 1
					# morgan.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8069 ):
				# pishella = find_npc_near( triggerer, 8069 )
				# if (pishella != OBJ_HANDLE_NULL):
					# game.global_vars[911] = game.global_vars[911] + 1
					# pishella.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8071 ):
				# rufus = find_npc_near( triggerer, 8071 )
				# if (rufus != OBJ_HANDLE_NULL):
					# game.global_vars[912] = game.global_vars[912] + 1
					# rufus.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8072 ):
				# spugnoir = find_npc_near( triggerer, 8072 )
				# if (spugnoir != OBJ_HANDLE_NULL):
					# game.global_vars[913] = game.global_vars[913] + 1
					# spugnoir.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8714 ):
				# holly = find_npc_near( triggerer, 8714 )
				# if (holly != OBJ_HANDLE_NULL):
					# game.global_vars[914] = game.global_vars[914] + 1
					# holly.float_line( 21000,triggerer )
			# if anyone( triggerer.group_list(), "has_follower", 8730 ):
				# ronald = find_npc_near( triggerer, 8730 )
				# if (ronald != OBJ_HANDLE_NULL):
					# game.global_vars[915] = game.global_vars[915] + 1
					# ronald.float_line( 21000,triggerer )
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (attachee.map == 5001):
		if (game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT
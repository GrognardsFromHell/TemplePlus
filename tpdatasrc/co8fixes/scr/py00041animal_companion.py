from toee import *
from utilities import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_start_combat( attachee, triggerer ):
	if (attachee.leader_get() != OBJ_HANDLE_NULL and not npc_get(attachee,2)):
		leader = attachee.leader_get()
		if (group_pc_percent_hp( attachee, leader ) <= 40):
			attachee.obj_set_int(obj_f_critter_strategy, 462)
		elif (game.party_npc_size() + game.party_pc_size() == 8):
			for pp in range(0,8):
				if (game.party[pp] != OBJ_HANDLE_NULL):
					if (obj_percent_hp(game.party[pp]) <= 50 and game.party[pp].stat_level_get(stat_hp_current) >= -9):
						game.global_flags[250 + pp] = 1
						game.global_flags[258] = 1

			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[253] == 1):
				if (adjacent(attachee, game.party[3])):
					game.global_flags[259] = 1
			if (game.global_flags[254] == 1):
				if (adjacent(attachee, game.party[4])):
					game.global_flags[259] = 1
			if (game.global_flags[255] == 1):
				if (adjacent(attachee, game.party[5])):
					game.global_flags[259] = 1
			if (game.global_flags[256] == 1):
				if (adjacent(attachee, game.party[6])):
					game.global_flags[259] = 1
			if (game.global_flags[257] == 1):
				if (adjacent(attachee, game.party[7])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 7):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[2]) <= 50 and game.party[2].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[252] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[3]) <= 50 and game.party[3].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[253] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[4]) <= 50 and game.party[4].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[254] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[5]) <= 50 and game.party[5].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[255] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[6]) <= 50 and game.party[6].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[256] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[253] == 1):
				if (adjacent(attachee, game.party[3])):
					game.global_flags[259] = 1
			if (game.global_flags[254] == 1):
				if (adjacent(attachee, game.party[4])):
					game.global_flags[259] = 1
			if (game.global_flags[255] == 1):
				if (adjacent(attachee, game.party[5])):
					game.global_flags[259] = 1
			if (game.global_flags[256] == 1):
				if (adjacent(attachee, game.party[6])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 6):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[2]) <= 50 and game.party[2].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[252] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[3]) <= 50 and game.party[3].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[253] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[4]) <= 50 and game.party[4].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[254] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[5]) <= 50 and game.party[5].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[255] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[253] == 1):
				if (adjacent(attachee, game.party[3])):
					game.global_flags[259] = 1
			if (game.global_flags[254] == 1):
				if (adjacent(attachee, game.party[4])):
					game.global_flags[259] = 1
			if (game.global_flags[255] == 1):
				if (adjacent(attachee, game.party[5])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 5):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[2]) <= 50 and game.party[2].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[252] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[3]) <= 50 and game.party[3].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[253] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[4]) <= 50 and game.party[4].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[254] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[253] == 1):
				if (adjacent(attachee, game.party[3])):
					game.global_flags[259] = 1
			if (game.global_flags[254] == 1):
				if (adjacent(attachee, game.party[4])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 4):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[2]) <= 50 and game.party[2].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[252] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[3]) <= 50 and game.party[3].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[253] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[253] == 1):
				if (adjacent(attachee, game.party[3])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 3):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[2]) <= 50 and game.party[2].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[252] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[252] == 1):
				if (adjacent(attachee, game.party[2])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_npc_size() + game.party_pc_size() == 2):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (obj_percent_hp(game.party[1]) <= 50 and game.party[1].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[251] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[251] == 1):
				if (adjacent(attachee, game.party[1])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
		elif (game.party_pc_size() == 1):
			if (obj_percent_hp(game.party[0]) <= 50 and game.party[0].stat_level_get(stat_hp_current) >= -9):
				game.global_flags[250] = 1
				game.global_flags[258] = 1
			if (game.global_flags[250] == 1):
				if (adjacent(attachee, game.party[0])):
					game.global_flags[259] = 1
			if (game.global_flags[258] == 1):
				if (game.global_flags[259] == 1):
					attachee.obj_set_int(obj_f_critter_strategy, 464)
					if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
						attachee.turn_towards(triggerer)
					else:
						for pc in game.party:
							if ( pc.has_feat(feat_animal_companion) ):
								attachee.turn_towards(pc)
							else:
								attachee.turn_towards(game.party[0])
				else:
					attachee.obj_set_int(obj_f_critter_strategy, 463)
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 464)
				if (triggerer.type == obj_t_npc and triggerer.leader_get() == OBJ_HANDLE_NULL):
					attachee.turn_towards(triggerer)
				else:
					for pc in game.party:
						if ( pc.has_feat(feat_animal_companion) ):
							attachee.turn_towards(pc)
						else:
							attachee.turn_towards(game.party[0])
	game.global_flags[250] = 0
	game.global_flags[251] = 0
	game.global_flags[252] = 0
	game.global_flags[253] = 0
	game.global_flags[254] = 0
	game.global_flags[255] = 0
	game.global_flags[256] = 0
	game.global_flags[257] = 0
	game.global_flags[258] = 0
	game.global_flags[259] = 0
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	if (npc_get(attachee,1)):
		npc_set(attachee,2)
	return RUN_DEFAULT


def san_spell_cast( attachee, triggerer, spell ):
	if ( spell.spell == spell_charm_person_or_animal or spell.spell == spell_charm_monster ):
		npc_set(attachee,1)
	return RUN_DEFAULT


def not_adjacent( companion, target ):
	if (companion.distance_to(target) >= 5):
		return 1
	return 0


def adjacent( companion, target ):
	if (companion.distance_to(target) <= 5):
		return 1
	return 0
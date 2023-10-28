from toee import *
from Co8 import Timed_Destroy
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (attachee.name != 20019):
		game.new_sid = 0
		return SKIP_DEFAULT
	triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT

def san_first_heartbeat( attachee, triggerer ):
	if (attachee.map != 5080 or game.global_flags[813] == 1):
		attachee.critter_flag_set( OCF_MUTE )
	game.new_sid = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	if (attachee.stat_level_get(stat_subdual_damage) >= 48 and game.global_flags[813] == 0 and attachee.map == 5080 and attachee.stat_level_get(stat_hp_current) >=1 and not game.combat_is_active()):
#		dice = dice_new("1d1+9")
#		attachee.healsubdual( OBJ_HANDLE_NULL, dice )
		for target in game.party[0].group_list():
			if (attachee.name == 14262):
				if (attachee.distance_to(target) <= 30 and target.type == obj_t_pc): 
					if (target.skill_level_get(attachee,skill_intimidate) >= 10 or target.skill_level_get(attachee,skill_diplomacy) >= 12 or target.skill_level_get(attachee,skill_bluff) >= 10):
						game.new_sid = 0
						dice = dice_new("1d1+9")
						attachee.healsubdual( OBJ_HANDLE_NULL, dice )
						attachee.ai_shitlist_remove( target )
						target.begin_dialog(attachee,1000)
						return SKIP_DEFAULT
					else:
						game.new_sid = 0
						dice = dice_new("1d1+9")
						attachee.healsubdual( OBJ_HANDLE_NULL, dice )
						attachee.ai_shitlist_remove( target )
						target.begin_dialog(attachee,2000)
						return SKIP_DEFAULT
	return RUN_DEFAULT


def san_exit_combat( attachee, triggerer ):
	if (attachee.stat_level_get(stat_subdual_damage) >= 48 and game.global_flags[813] == 0 and attachee.map == 5080 and attachee.stat_level_get(stat_hp_current) >=1 and not game.combat_is_active()):
#		dice = dice_new("1d1+9")
#		attachee.healsubdual( OBJ_HANDLE_NULL, dice )
		for target in game.party[0].group_list():
			if (attachee.name == 14262):
				if (attachee.distance_to(target) <= 30 and target.type == obj_t_pc): 
					if (target.skill_level_get(attachee,skill_intimidate) >= 10 or target.skill_level_get(attachee,skill_diplomacy) >= 12 or target.skill_level_get(attachee,skill_bluff) >= 10):
						game.new_sid = 0
						dice = dice_new("1d1+9")
						attachee.healsubdual( OBJ_HANDLE_NULL, dice )
						attachee.ai_shitlist_remove( target )
						target.begin_dialog(attachee,1000)
						return SKIP_DEFAULT
					else:
						game.new_sid = 0
						dice = dice_new("1d1+9")
						attachee.healsubdual( OBJ_HANDLE_NULL, dice )
						attachee.ai_shitlist_remove( target )
						target.begin_dialog(attachee,2000)
						return SKIP_DEFAULT
	return RUN_DEFAULT

def run_off( attachee, triggerer ):
#	for pc in game.party:
#		attachee.ai_shitlist_remove( pc )
#		attachee.reaction_set( pc, 50 )
	attachee.runoff(attachee.location-3)
	Timed_Destroy(attachee, 5000)
	return RUN_DEFAULT

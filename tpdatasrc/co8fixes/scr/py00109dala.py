from utilities import *
from toee import *
from py00439script_daemon import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if get_f('dala_ran_off'):
		triggerer.begin_dialog( attachee, 200)
	elif (game.global_flags[88] == 1):
		triggerer.begin_dialog( attachee, 1 )
	elif (game.quests[37].state == qs_completed):
		triggerer.begin_dialog( attachee, 20 )
	elif (attachee.has_met( triggerer )):
		triggerer.begin_dialog( attachee, 60 )
	else:
		triggerer.begin_dialog( attachee, 110 )
	return SKIP_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) == 0 and not attachee.d20_query(Q_Prone)):
		run_off( attachee, triggerer )
		#attachee.float_mesfile_line( 'mes\\test.mes', 4, 0 ) 
		return SKIP_DEFAULT
		dummy = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) == 0 and not attachee.d20_query(Q_Prone)):
		run_off( attachee, triggerer )
		#attachee.float_mesfile_line( 'mes\\test.mes', 3, 0 ) 
		return SKIP_DEFAULT
		dummy = 1
	return RUN_DEFAULT	


def san_heartbeat( attachee, triggerer ):
	attachee.scripts[13] = 109 #assign enter_combat script
	if (game.combat_is_active()):
		if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
			run_off( attachee, triggerer )
			return SKIP_DEFAULT
	if (not game.combat_is_active() and tpsts('dala_buggered_off', 1)):
		downed_bozos = 0
		for hostel_patron in game.obj_list_vicinity(attachee.location, OLC_NPC):
			if hostel_patron.name in [8018, 14145, 14074]:
				if hostel_patron.is_unconscious() == 1 or critter_is_unconscious( hostel_patron ) or hostel_patron.leader_get() != OBJ_HANDLE_NULL:
					downed_bozos += 1
		if downed_bozos >= 2:
			#attachee.float_mesfile_line( 'mes\\test.mes', 1, 0 ) 
			attachee.fade_to( 255, 1, 50)
			for hostel_patron in game.obj_list_vicinity(attachee.location, OLC_NPC):
				for pc in game.leader.group_list():
					hostel_patron.ai_shitlist_remove(pc)
					hostel_patron.reaction_set( pc, 80 )
			attachee.object_flag_unset(OF_CLICK_THROUGH)
			attachee.object_flag_unset(OF_OFF)
			for hostel_patron in game.obj_list_vicinity(attachee.location, OLC_NPC):
				for pc in game.leader.group_list():
					hostel_patron.ai_shitlist_remove(pc)
					hostel_patron.reaction_set( pc, 80 )
			attachee.stat_base_set(stat_strength, 10)
			if get_f('have_talked_to_dala_post_battle') == 0: # initiate Dala monologue where she faints
				for pc in game.obj_list_vicinity(attachee.location,OLC_PC):
					if is_safe_to_talk_rfv(attachee, pc, radius = 40, facing_required = 0, visibility_required = 1):
						set_f('have_talked_to_dala_post_battle')
						pc.begin_dialog( attachee, 200)
	elif (not game.combat_is_active()  ): # this takes care of the infinite battle loop
		#attachee.float_mesfile_line( 'mes\\test.mes', 2, 0 ) 
		for hostel_patron in game.obj_list_vicinity(attachee.location, OLC_NPC):
			if not hostel_patron.name in [8018, 14145, 14074]: # added condition because apparently sometimes combat doesn't start before this heartbeat fires and thus it sets them to non-hostile status and no combat actually commences
				for pc in game.leader.group_list():
					hostel_patron.ai_shitlist_remove(pc)
					hostel_patron.reaction_set( pc, 80 )
	if (game.quests[37].state == qs_completed):
		#game.new_sid = 0 # commented by S.A. - the heartbeat is now needed
		dummy = 1
	elif (game.global_flags[89] == 0):
		if (not game.combat_is_active()):
			xx, yy = location_to_axis(attachee.location)
			if xx == 478 and yy == 504 and not attachee.can_see(game.leader):
				attachee.turn_towards(game.leader)
			for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
				if (attachee.can_see(obj)):
					game.global_flags[89] = 1
					game.timevent_add( reset_global_flag_89, ( attachee, ), 7200000 ) # call reset_global_flag_89 in 2 hours
					attachee.steal_from(obj)
					return RUN_DEFAULT
	return RUN_DEFAULT


def san_caught_thief( attachee, triggerer ):
	triggerer.begin_dialog( attachee, 120 )
	return RUN_DEFAULT


def reset_global_flag_89( attachee ):
	game.global_flags[89] = 0
	return RUN_DEFAULT


def make_dick_talk( attachee, triggerer, line):
	npc = find_npc_near(attachee,8018)
	if (npc != OBJ_HANDLE_NULL):
		triggerer.begin_dialog(npc,line)
		npc.turn_towards(attachee)
		attachee.turn_towards(npc)
	else:
		triggerer.begin_dialog(attachee,130)
	return SKIP_DEFAULT


def run_off( attachee, triggerer ):
	lfa = location_from_axis(501,490)
	attachee.fade_to( 0, 1, 15)

	# attachee.condition_add_with_args( "prone", 0, 0 )

	#attachee.critter_flag_set(OCF_SLEEPING)
	#attachee.critter_flag_set(OCF_BLINDED)
	
	# prevent her from taking AoO 
	attachee.stat_base_set(stat_strength, -5)
	attachee.condition_add_with_args( 'Paralyzed - ability score', 0, 2, 0 )	
	game.timevent_add(set_to_of_off, ( attachee ), 1600 , 1 )
	game.timevent_add(set_to_of_off, ( attachee ), 3500 , 1 ) # call it a 2nd time because it takes time for striking/casting on her to register
	if tpsts('dala_buggered_off', 0) == 0:
		attachee.float_line(220, triggerer)
	#attachee.runoff(lfa)
	#attachee.scripts[19] = 109
	return RUN_DEFAULT
	
def set_to_of_off( obj):
	if (obj != OBJ_HANDLE_NULL and critter_is_unconscious(obj) != 1 and obj.leader_get()==OBJ_HANDLE_NULL and not obj.d20_query(Q_Critter_Is_Held) ): # mainly in case the player initiates the fight with her e.g. one-shotting or charming her etc.
			#and not obj.d20_query(Q_Prone) # removed this one - I think it's reasonable that she crawls under something even after being tripped
		obj.object_flag_set(OF_CLICK_THROUGH)
		#obj.move(obj.location + 50 , 0.0, 0.0)
		record_time_stamp('dala_buggered_off')
		obj.object_flag_set(OF_OFF)	
	else: 
		obj.object_flag_unset(OF_CLICK_THROUGH)
		#obj.move(obj.location - 50 , 0.0, 0.0)
		obj.object_flag_unset(OF_OFF)	
		obj.fade_to( 255, 1, 35)
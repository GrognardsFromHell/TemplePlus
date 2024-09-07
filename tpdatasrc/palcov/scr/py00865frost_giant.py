from toee import *
from scripts import *
from utilities import *
from marc import *
from triggers import *

def san_dialog (attachee, triggerer):
	return SKIP_DEFAULT

def san_first_heartbeat (attachee, triggerer):
	float_mes(attachee,100)
	if getbit_1(attachee,0) == 0:  # inventory
		restock (attachee, 0, 0, 0, 1)
		npcbit_1(attachee,0)
	if attachee.leader_get() == OBJ_HANDLE_NULL:
		set_age(attachee)
		san_first_heartbeat_triggers (attachee, triggerer)
	return RUN_DEFAULT

def san_heartbeat (attachee, triggerer):
	float_mes(attachee,101)
	if attachee.leader_get() == OBJ_HANDLE_NULL:
		san_heartbeat_triggers (attachee, triggerer)
	return RUN_DEFAULT

def san_enter_combat (attachee, triggerer):
	float_mes(attachee,102)
	san_enter_combat_special (attachee, triggerer)
	if attachee.leader_get() == OBJ_HANDLE_NULL:
		san_enter_combat_triggers (attachee, triggerer)
	return RUN_DEFAULT

def san_start_combat (attachee, triggerer):
	float_mes(attachee,104)

	if not attachee.is_unconscious():
		if attachee.obj_get_int(obj_f_critter_gender) == gender_male:
			game.sound(663,1)
		else:
			game.sound(4708,1)

	san_start_combat_special (attachee, triggerer)

	# B1, chapter 4, call out to frost chief and frost giantesses in nearby rooms
	if game.quests[73].state == qs_accepted:
		if attachee.name in (14164,14165):
			for obj in game.obj_list_cone (attachee, OLC_CRITTERS, 60, 0, 359):
				if obj.name in (14164,14165):
					obj.attack(game_leader())
					for pc in game.party:
						obj.attack(pc)

	return RUN_DEFAULT

def san_dying (attachee, triggerer):
	float_mes(attachee,106)
	san_dying_special (attachee, triggerer, 0, 0)
	san_dying_triggers (attachee, triggerer)
	return RUN_DEFAULT

def frost_giant_and_friends_on (npc, pc):
	game.sound(4380,1)
	gal = group_average_level(pc)
	for critter in game.obj_list_cone (npc, OLC_NPC, 80, 0, 360):
		if critter.leader_get() == OBJ_HANDLE_NULL:
			if (critter.name in (8234, 14868)        # frost giant talker, mountain gnolls
			  or (critter.name == 14867 and gal >= 2)  # cold hounds
			  or (critter.name == 14712 and gal >= 4)  # gnoll shaman
			  or (critter.name == 14262 and gal >= 5)  # troll
			  or (critter.name == 14865 and gal >= 8)  # frost giant #2
			  ):
				critter.object_flag_unset(OF_CLICK_THROUGH)
				critter.object_flag_unset(OF_DONTDRAW)
				game.particles ("sp-Gaseous Form-END", critter)
				game.particles ("sp-polymorph other", critter)
			elif critter.name in (8234,14867,14868,14712,14262,14865):
				critter.object_flag_set(OF_OFF)

def frost_giant_departs (npc, pc):
	game.particles ("sp-Gaseous Form-END", npc)
	game.particles ("sp-polymorph other", npc)
	npc.object_flag_set(OF_CLICK_THROUGH)
	npc.object_flag_set(OF_DONTDRAW)
	#npc.object_flag_set(OF_OFF)
	game.sound(4379,1)
	game.sound(4380,1)
	game.shake(50,3200)

def set_kos (npc, pc):
	for critter in game.obj_list_cone (pc, OLC_NPC, 100, 0, 360):
		if not critter.object_flags_get() & OF_CLICK_THROUGH:
			if critter.leader_get() == OBJ_HANDLE_NULL:
				if critter.name in (8234,14867,14868,14712,14262,14865):
					critter.npc_flag_set(ONF_KOS)
	npc.destroy()


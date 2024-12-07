from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if ( game.global_vars[2] >= 100 and triggerer.reputation_has( 3 ) == 0 ):
		triggerer.reputation_add( 3 )
	attachee.turn_towards(triggerer)
	if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6):
		triggerer.begin_dialog( attachee, 630 )
	elif (game.global_vars[501] == 2):
		triggerer.begin_dialog( attachee, 580 )
	else:
		triggerer.begin_dialog( attachee, 1 )
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	return RUN_DEFAULT
	if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
		game.global_vars[722] = 0
	if (game.global_vars[510] == 2):
		attachee.object_flag_set(OF_OFF)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[337] = 1
	game.global_vars[23] = game.global_vars[23] + 1
	if (game.global_vars[23] >= 2):
		game.party[0].reputation_add( 92 )
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	# if (not attachee.has_wielded(4047) or not attachee.has_wielded(4111)):
	if (  not attachee.has_wielded(4111)  ):	# 4111 (Rod of the Python) is a two-handed quarterstaff since v6.1
		attachee.item_wield_best_all()
	if anyone( triggerer.group_list(), "has_follower", 8000 ):
		elmo = find_npc_near( triggerer, 8000 )
		if (elmo != OBJ_HANDLE_NULL):
			triggerer.follower_remove(elmo)
			elmo.float_line( 12021,triggerer )
			elmo.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8001 ):
		paida = find_npc_near( triggerer, 8001 )
		if (paida != OBJ_HANDLE_NULL):
			triggerer.follower_remove(paida)
			paida.float_line( 12021,triggerer )
			paida.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8014 ):
		otis = find_npc_near( triggerer, 8014 )
		if (otis != OBJ_HANDLE_NULL):
			triggerer.follower_remove(otis)
			otis.float_line( 12021,triggerer )
			otis.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8015 ):
		meleny = find_npc_near( triggerer, 8015 )
		if (meleny != OBJ_HANDLE_NULL):
			triggerer.follower_remove(meleny)
			meleny.float_line( 12021,triggerer )
			meleny.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8021 ):
		ydey = find_npc_near( triggerer, 8021 )
		if (ydey != OBJ_HANDLE_NULL):
			triggerer.follower_remove(ydey)
			ydey.float_line( 12021,triggerer )
			ydey.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8022 ):
		murfles = find_npc_near( triggerer, 8022 )
		if (murfles != OBJ_HANDLE_NULL):
			triggerer.follower_remove(murfles)
			murfles.float_line( 12021,triggerer )
			murfles.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8031 ):
		thrommel = find_npc_near( triggerer, 8031 )
		if (thrommel != OBJ_HANDLE_NULL):
			triggerer.follower_remove(thrommel)
			thrommel.float_line( 12021,triggerer )
			thrommel.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8039 ):
		taki = find_npc_near( triggerer, 8039 )
		if (taki != OBJ_HANDLE_NULL):
			triggerer.follower_remove(taki)
			taki.float_line( 12021,triggerer )
			taki.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8054 ):
		burne = find_npc_near( triggerer, 8054 )
		if (burne != OBJ_HANDLE_NULL):
			triggerer.follower_remove(burne)
			burne.float_line( 12021,triggerer )
			burne.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8060 ):
		morgan = find_npc_near( triggerer, 8060 )
		if (morgan != OBJ_HANDLE_NULL):
			triggerer.follower_remove(morgan)
			morgan.float_line( 12021,triggerer )
			morgan.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8069 ):
		pishella = find_npc_near( triggerer, 8069 )
		if (pishella != OBJ_HANDLE_NULL):
			triggerer.follower_remove(pishella)
			pishella.float_line( 12021,triggerer )
			pishella.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8071 ):
		rufus = find_npc_near( triggerer, 8071 )
		if (rufus != OBJ_HANDLE_NULL):
			triggerer.follower_remove(rufus)
			rufus.float_line( 12021,triggerer )
			rufus.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8072 ):
		spugnoir = find_npc_near( triggerer, 8072 )
		if (spugnoir != OBJ_HANDLE_NULL):
			triggerer.follower_remove(spugnoir)
			spugnoir.float_line( 12021,triggerer )
			spugnoir.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8714 ):
		holly = find_npc_near( triggerer, 8714 )
		if (holly != OBJ_HANDLE_NULL):
			triggerer.follower_remove(holly)
			holly.float_line( 1000,triggerer )
			holly.attack(triggerer)
	if anyone( triggerer.group_list(), "has_follower", 8730 ):
		ronald = find_npc_near( triggerer, 8730 )
		if (ronald != OBJ_HANDLE_NULL):
			triggerer.follower_remove(ronald)
			ronald.float_line( 12021,triggerer )
			ronald.attack(triggerer)
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	print "Jaroo start combat"
	if (not attachee.has_wielded(4047) or not attachee.has_wielded(4111)):
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[337] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[875] == 1 and game.global_flags[876] == 0 and game.quests[99].state != qs_completed and not anyone( triggerer.group_list(), "has_item", 12900 )):
		game.global_flags[876] = 1
		game.timevent_add( amii_dies, (), 140000000 )
	game.global_vars[722] = game.global_vars[722] + 1
	if (game.combat_is_active()) or (game.party_alignment & ALIGNMENT_GOOD) != 0:
		return RUN_DEFAULT
	if (game.global_vars[722] == 1):
		attachee.cast_spell(spell_stoneskin, attachee)
		attachee.spells_pending_to_memorized()	
	#if (game.global_vars[722] >= 5 and (not attachee.has_wielded(4047) or not attachee.has_wielded(4111))):
	if (game.global_vars[722] >= 5 and not attachee.has_wielded(4111)  ):	# 4111 (Rod of the Python) is a two-handed quarterstaff since v6.1
		attachee.item_wield_best_all()
		#attachee.item_wield_best_all()
#	if (not attachee.has_spell_effects()):
#		game.global_vars[722] = 0
	return RUN_DEFAULT


def amii_dies_due_to_dont_give_a_damn_dialog( attachee, triggerer ):
	game.global_flags[876] = 1
	game.timevent_add( amii_dies, (), 140000000 )
	return RUN_DEFAULT


def amii_dies():
	game.quests[99].state = qs_botched
	game.global_flags[862] = 1
	return RUN_DEFAULT


# Function: should_clear_spell_on( obj )
# Author  : Livonya
# Returns : 0 if obj is dead, else 1
# Purpose : to fix only characters that need it


def should_clear_spell_on( obj ):
	if (obj.stat_level_get(stat_hp_current) <= -10):
		return 0
	return 1


def kill_picked( obj, jaroo ):
	tempp = game.global_vars[23]
	damage_dice = dice_new( '100d20' )
	obj.damage( OBJ_HANDLE_NULL, 0, damage_dice )

	# simulate reincarnate casting
	jaroo.anim_goal_push(94) # conjuration casting
	game.particles('sp-Raise Dead', obj)
	game.sound_local_obj(17342, obj) # reincarnate sound
	obj.resurrect(CRITTER_R_CUTHBERT_RESURRECT, 0)

	if tempp <= 1:
		game.timevent_add( de_butcherize, (tempp), 500, 1 )
	return RUN_DEFAULT

def de_butcherize( tempp ):
	game.leader.reputation_remove(1)
	game.global_vars[23] = tempp


def run_off(npc, pc):
	game.quests[99].state = qs_completed
	for obj in game.obj_list_vicinity(npc.location,OLC_NPC):
		if (obj.name == 8090):
			obj.runoff(obj.location+3)
	return RUN_DEFAULT
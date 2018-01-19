from toee import *
from utilities import *
from combat_standard_routines import *


def san_dialog( attachee, triggerer ):
	if (game.global_flags[819] == 1):
		attachee.attack(triggerer)
		return SKIP_DEFAULT
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		triggerer.begin_dialog( attachee, 100 )			## spugnoir in party
	elif (game.global_vars[913] == 32):
		triggerer.begin_dialog( attachee, 140 )			## have attacked 3 or more farm animals with spugnoir in party
	elif (game.leader.reputation_has(32) == 1 or game.leader.reputation_has(30) == 1 or game.leader.reputation_has(29) == 1):
		attachee.float_line(11004,triggerer)			## have lawbreaker or convict or banished from hommlet rep
	else:
		triggerer.begin_dialog( attachee, 1 )			## none of the above
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.leader_get() == OBJ_HANDLE_NULL):
		if (game.global_vars[501] == 4 or game.global_vars[501] == 5 or game.global_vars[501] == 6 or game.global_vars[510] == 2):
			attachee.object_flag_set(OF_OFF)
		else:
			attachee.object_flag_unset(OF_OFF)
			if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
				game.global_vars[712] = 0
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	attachee.float_line(12014,triggerer)
	if (attachee.leader_get() != OBJ_HANDLE_NULL):
		game.global_vars[29] = game.global_vars[29] + 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	print "Spugnoir Enter Combat"
	ProtectTheInnocent( attachee, triggerer)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_vars[712] == 0 and attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()) and game.party_alignment & ALIGNMENT_GOOD == 0:
		attachee.cast_spell(spell_mage_armor, attachee)
		attachee.spells_pending_to_memorized()
		game.global_vars[712] = 1
	if (not game.combat_is_active()):
		if (game.global_vars[913] >= 3):
			if (attachee != OBJ_HANDLE_NULL):
				leader = attachee.leader_get()
				if (leader != OBJ_HANDLE_NULL):
					leader.follower_remove(attachee)
					attachee.float_line(22000,triggerer)
	return RUN_DEFAULT


def san_join( attachee, triggerer ):
	create_item_in_inventory( 4645, attachee)
	create_item_in_inventory( 4647, attachee)
	create_item_in_inventory( 4224, attachee)
	create_item_in_inventory( 12848, attachee)
	game.new_sid = 0
	return RUN_DEFAULT


def san_new_map( attachee, triggerer ):
	if ((attachee.map == 5006) and (game.global_vars[695] == 1 or game.global_vars[695] == 2)):
		attachee.float_line(12070,triggerer)
	elif ((attachee.map == 5024) and (is_daytime() != 1)):
		attachee.float_line(10019,triggerer)
	return RUN_DEFAULT


def equip_transfer( attachee, triggerer ):
	itemA = attachee.item_find(6081)
	if (itemA != OBJ_HANDLE_NULL):
		itemA.destroy()
		create_item_in_inventory( 6081, triggerer )
	itemB = attachee.item_find(6023)
	if (itemB != OBJ_HANDLE_NULL):
		itemB.destroy()
		create_item_in_inventory( 6023, triggerer )
	itemC = attachee.item_find(4060)
	if (itemC != OBJ_HANDLE_NULL):
		itemC.destroy()
		create_item_in_inventory( 4060, triggerer )
	create_item_in_inventory( 7001, attachee )
	return RUN_DEFAULT
from toee import *
from utilities import *
from combat_standard_routines import *


def san_first_heartbeat( attachee, triggerer ):
	if (attachee.name == 8075):
	# hedrack ally mage 1
		if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
			game.global_vars[747] = 0
	elif (attachee.name == 8076):
	# hedrack ally mage 2
		if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
			game.global_vars[748] = 0
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (game.global_flags[544] == 1):
		return SKIP_DEFAULT
	elif ((attachee.name == 8083 or attachee.name == 8084) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
	# ettin two weapon fighting script
		attachee.item_wield_best_all()
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	if ((attachee.name == 8083 or attachee.name == 8084) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
	# ettin two weapon fighting script
		attachee.item_wield_best_all()
	elif (attachee.name == 8075):
	# hedrack ally mage 1
		if (obj_percent_hp(attachee) <= 66):
			if (game.global_vars[745] == 0):
				attachee.obj_set_int(obj_f_critter_strategy, 471)
				game.global_vars[745] = game.global_vars[745] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 466)
		else:
			attachee.obj_set_int(obj_f_critter_strategy, 466)
	elif (attachee.name == 8076):
	# hedrack ally mage 2
		if (obj_percent_hp(attachee) <= 66):
			if (game.global_vars[746] == 0):
				attachee.obj_set_int(obj_f_critter_strategy, 471)
				game.global_vars[746] = game.global_vars[746] + 1
			else:
				attachee.obj_set_int(obj_f_critter_strategy, 467)
		else:
			attachee.obj_set_int(obj_f_critter_strategy, 467)
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[544] == 1): # Hedrack battle has paused for talking, no one will KOS
		for pc in game.party:
			attachee.ai_shitlist_remove( pc )
	if (attachee != OBJ_HANDLE_NULL and critter_is_unconscious(attachee) != 1 and not attachee.d20_query(Q_Prone)):
		if ((attachee.name == 8083 or attachee.name == 8084) and (not attachee.has_wielded(4099) or not attachee.has_wielded(4100))):
		# ettin two weapon fighting script
			attachee.item_wield_best_all()
			attachee.item_wield_best_all()
			game.new_sid = 0
		elif (attachee.name == 8075):
		# hedrack ally mage 1
			closest_jones = party_closest(attachee)
			if ( attachee.distance_to(closest_jones) <= 100 ):
				game.global_vars[747] = game.global_vars[747] + 1
				if (attachee.leader_get() == OBJ_HANDLE_NULL):
					if (game.global_vars[747] == 4):
						attachee.cast_spell(spell_mage_armor, attachee)
						attachee.spells_pending_to_memorized()
					if (game.global_vars[747] == 8):
						attachee.cast_spell(spell_protection_from_arrows, attachee)
						attachee.spells_pending_to_memorized()
				if (game.global_vars[747] >= 400):
					game.global_vars[747] = 0
		elif (attachee.name == 8076):
		# hedrack ally mage 2
			if any([attachee.distance_to(pc) <= 100 for pc in game.party]):
				game.global_vars[748] = game.global_vars[748] + 1
				if (attachee.leader_get() == OBJ_HANDLE_NULL):
					if (game.global_vars[748] == 4):
						attachee.cast_spell(spell_mage_armor, attachee)
						attachee.spells_pending_to_memorized()
					if (game.global_vars[748] == 8):
						attachee.cast_spell(spell_protection_from_arrows, attachee)
						attachee.spells_pending_to_memorized()
				if (game.global_vars[748] >= 400):
					game.global_vars[748] = 0
	return RUN_DEFAULT


def will_kos( attachee, triggerer ):
	if (game.global_flags[544] == 1):
		return SKIP_DEFAULT
	return RUN_DEFAULT
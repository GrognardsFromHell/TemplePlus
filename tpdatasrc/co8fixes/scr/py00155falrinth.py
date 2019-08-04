from utilities import *
from combat_standard_routines import *
from toee import *
import _include
from co8Util.PersistentData import *
from co8Util.ObjHandling import *
from Co8 import StopCombat
MINOR_GLOBE_OF_INVULNERABILITY_KEY = "Sp311_MINOR_GLOBE_OF_INVULNERABILITY_Activelist"


def san_dialog( attachee, triggerer ):
	if (not attachee.has_met(triggerer)):
		triggerer.begin_dialog( attachee, 1 )
	elif (game.global_vars[901] == 2):
		return RUN_DEFAULT
	elif (game.global_flags[164] == 1):
		triggerer.begin_dialog( attachee, 130 )
	else:
		return RUN_DEFAULT
	return SKIP_DEFAULT


def san_first_heartbeat( attachee, triggerer ):
	if (game.global_flags[372] == 1):
		attachee.object_flag_set(OF_OFF)
	else:
		if (attachee.leader_get() == OBJ_HANDLE_NULL and not game.combat_is_active()):
			game.global_vars[721] = 0
			StopCombat(attachee, 1)
	return RUN_DEFAULT


def san_dying( attachee, triggerer ):
	if should_modify_CR( attachee ):
		modify_CR( attachee, get_av_level() )
	game.global_flags[335] = 1
	return RUN_DEFAULT


def san_enter_combat( attachee, triggerer ):
	if (game.global_flags[821] == 0):
		create_item_in_inventory( 8904, attachee )
		game.global_vars[763] = 0
		create_item_in_inventory( 9173, attachee )
		game.global_flags[821] = 1
	return RUN_DEFAULT


def san_start_combat( attachee, triggerer ):
	leader = game.leader
	game.global_vars[763] = game.global_vars[763] + 1
	if (game.global_vars[763] == 2 or game.global_vars[763] == 35):
		create_item_in_inventory( 8900, attachee )
		create_item_in_inventory( 8037, attachee )
	if (game.global_vars[763] == 3 or game.global_vars[763] == 40):
		create_item_in_inventory( 8901, attachee )
	if (game.global_vars[763] == 8):
		create_item_in_inventory( 8902, attachee )
	if (game.global_vars[763] == 11 or game.global_vars[763] == 30):
		create_item_in_inventory( 8904, attachee )
	if (game.global_flags[164] == 1 and game.global_vars[901] == 2):
		return RUN_DEFAULT
#		game.new_sid = 0
	elif (obj_percent_hp(attachee) < 50):
		for pc in game.party:
			#if pc.type == obj_t_pc:
			attachee.ai_shitlist_remove( pc )
#		game.global_flags[822] = 1
		if (game.global_vars[901] == 0):
			leader.begin_dialog( attachee, 200 )
			return SKIP_DEFAULT
		elif (game.global_vars[901] == 1):
			leader.begin_dialog( attachee, 130 )
			return SKIP_DEFAULT
	return RUN_DEFAULT


def san_resurrect( attachee, triggerer ):
	game.global_flags[335] = 0
	return RUN_DEFAULT


def san_heartbeat( attachee, triggerer ):
	if (game.global_flags[821] == 0):
		create_item_in_inventory( 8904, attachee )
		game.global_vars[763] = 0
		create_item_in_inventory( 9173, attachee )
		game.global_flags[821] = 1
	if (game.combat_is_active()):
		return RUN_DEFAULT
	if (attachee.item_find(4060) != OBJ_HANDLE_NULL and attachee.leader_get() == OBJ_HANDLE_NULL):
		itemA = attachee.item_find(4060)
		itemA.destroy()
		create_item_in_inventory ( 4177, attachee )
		create_item_in_inventory ( 5011, attachee )
		create_item_in_inventory ( 5011, attachee )
		create_item_in_inventory ( 5011, attachee )
	if (not game.combat_is_active()):
		for obj in game.obj_list_vicinity(attachee.location,OLC_PC):
			if (not attachee.has_met(obj)):
				if (is_safe_to_talk(attachee,obj)):
					obj.turn_towards(attachee)	## added by Livonya
					attachee.turn_towards(obj)	## added by Livonya
					obj.begin_dialog(attachee,1)
#					game.new_sid = 0			## removed by Livonya
	if (game.global_vars[721] == 0):
		attachee.cast_spell(spell_foxs_cunning, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[721] == 4):
		attachee.cast_spell(spell_cats_grace, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[721] == 8):
		loc = attachee.location
		attachee.cast_spell(spell_heroism, attachee)
		attachee.spells_pending_to_memorized()
	if (game.global_vars[721] == 12):
		attachee.cast_spell(spell_mage_armor, attachee)
		attachee.spells_pending_to_memorized()	
	game.global_vars[721] = game.global_vars[721] + 1
	return RUN_DEFAULT


def san_will_kos( attachee, triggerer ):
	if (game.global_flags[164] == 1):
		return SKIP_DEFAULT
	return RUN_DEFAULT


def falrinth_escape( attachee, triggerer ):
	attachee.object_flag_set(OF_OFF)
	game.global_vars[901] = 1	## added by Gaear
	game.timevent_add( falrinth_return, ( attachee, ), 43200000 )	## 43200000ms is 12 hours
	#activeList = Co8PersistentData.getData(MINOR_GLOBE_OF_INVULNERABILITY_KEY)
	#if isNone(activeList):
	#	print "ERROR! Active Globe spell without activeList!"
	#	return
	#
	#for entry in activeList:
	#	spellID, target = entry
	#	targetObj = refHandle(target)
	#	if spellID == targetObj.id:
	#		targetObj.spell_end( targetObj.id )
	return RUN_DEFAULT


def falrinth_return( attachee ):
#	game.global_flags[164] = 0	## removed by Gaear
	attachee.spells_pending_to_memorized()	## added by Livonya
	attachee.object_flag_unset(OF_OFF)
	game.particles( "sp-Dimension Door", attachee )
	game.sound( 4018, 1 )
	return RUN_DEFAULT


def falrinth_well( attachee, pc ):
	dice = dice_new("1d10+1000")
	attachee.heal( OBJ_HANDLE_NULL, dice )
	attachee.healsubdual( OBJ_HANDLE_NULL, dice )
	return RUN_DEFAULT
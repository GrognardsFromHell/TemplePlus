from toee import *
from __main__ import game
from utilities import *
from Co8 import *
#from itt import *
from array import *
from math import atan2
from py00439script_daemon import *
from quickstart_module import *
from teleport_shortcuts import *

## Last update 2010 - 09 - 30


ohn = OBJ_HANDLE_NULL
ggv = game.global_vars
ggf = game.global_flags

def dmp():
	import templeplus.debug.dump_conditions
#	import templeplus.debug.dump_d20actions
#	import templeplus.debug.dump_feats
	templeplus.debug.dump_conditions.run()
#	templeplus.debug.dump_d20actions.run()
#	templeplus.debug.dump_feats.run()
	import templeplus.debug.dump_ai_tactics
	templeplus.debug.dump_ai_tactic_defs.run()
	return



	
def tiuz(noiuz = 0):
	if noiuz == 0:
		game.global_vars[697] = 1
	game.fade_and_teleport( 0,0,0,5121,509,652 )


def tf():
	obj = s(14000 + game.global_vars[998])
	print str( obj.obj_get_int(obj_f_npc_pad_i_3) ) + " " + str( obj.obj_get_int(obj_f_npc_pad_i_4) ) + " " + str( obj.obj_get_int(obj_f_npc_pad_i_5) )
	if obj.obj_get_int(obj_f_npc_pad_i_3) != 0 or obj.obj_get_int(obj_f_npc_pad_i_4) != 0 or obj.obj_get_int(obj_f_npc_pad_i_5) != 0:
		print "   " + str(14000 + game.global_vars[998]) + "  " + str(obj)
	else:
		obj.destroy()
	game.global_vars[998] += 1
	return obj



speedup() # see batch.py; imports preference from speedup.ini


def t_mode():
	try:
		ff_t_mode = open('modules\\ToEE\\test_mode.ini', 'r')
		asdf = ff_t_mode.readline()
		while asdf != '':
			asdf_s = asdf.split('=')
			asdf_s[0] = asdf_s[0].strip().lower()
			asdf_s[1] = asdf_s[1].strip().lower()
		
			if asdf_s[0] == 'Test_Mode_Enabled'.lower().strip():
				# Enables flag 403, which is sort of a master switch for a lot of things
				if asdf_s[1] == '1':
					game.global_flags[403] = 1
				else: 
					game.global_flags[403] = 0

			elif asdf_s[0] == 'Random_Encounters_Disabled'.lower().strip():
				if asdf_s[1] == '1':
					set_f('qs_disable_random_encounters')
				else:
					set_f('qs_disable_random_encounters', 0)

			elif asdf_s[0] == 'Quickstart_Autoloot_Enabled'.lower().strip():
				if asdf_s[1] == '1':
					set_f('qs_autoloot', 1)
				else:
					set_f('qs_autoloot', 0)

			elif asdf_s[0] == 'Quickstart_Autoloot_AutoConvert_Jewels_Enabled'.lower().strip():
				if asdf_s[1] == '1':
					set_f('qs_autoconvert_jewels', 1)
				else:
					set_f('qs_autoconvert_jewels', 0)
			asdf = ff_t_mode.readline()
		ff_t_mode.close()
	finally:
		dummy = 1
	return

t_mode()

def list_flags():
	ff = open('flag_list.txt', 'w')
	f_lines = ''
	for pp in range(0, 999):
		if game.global_flags[pp] == 1:
			f_lines = f_lines + str(pp) + '\n'
			print str(pp)
	ff.write(f_lines)
	ff.close()
	return

def list_vars():
	ff = open('var_list.txt', 'w')
	f_lines = ''
	for pp in range(0, 999):
		if game.global_vars[pp] != 0:
			f_lines = f_lines + str(pp) + '=' + str(game.global_vars[pp] ) + '\n'
			print str(pp) + '=' + str(game.global_vars[pp] )
	ff.write(f_lines)
	ff.close()
	return


def list_quests():
	ff = open('completed_quest_list.txt', 'w')
	f_lines = ''
	for pp in range(0, 999):
		if game.quests[pp].state == qs_completed:
			f_lines = f_lines + str(pp) + '=' + str(game.quests[pp].state ) + '\n'
			print str(pp) + '=' + str(game.quests[pp].state )
	ff.write(f_lines)
	ff.close()
	return



def restup():
	for pc in game.party[0].group_list():
		pc.spells_pending_to_memorized() # Memorizes Spells
		pc.obj_set_int( 29, 0) # Removes all damage (doesn't work for companions?)

def cnk(proto_id, do_not_destroy = 0, how_many = 1, timer = 0):
	# Create n' Kill
	# Meant to simulate actually killing the critter
	#if timer == 0:
	for pp in range(0, how_many):
		a = s(proto_id)
		damage_dice = dice_new( '50d50' )
		a.damage( game.party[0], 0, damage_dice )
		if do_not_destroy != 1:
			a.destroy()
	#else:
	#	for pp in range(0, how_many):
	#		game.timevent_add( cnk, (proto_id, do_not_destroy, 1, 0), (pp+1)*20 )
	return

def idall():
	for pc in game.party:
		pc.identify_all()
	return

def hpav(force_av = 0):
	# Checks for below avg HP
	# If HP is below avg, sets it to avg
	# If force_av is NOT 0, it forces average HP
	for pc in game.party:
		lev_barb = pc.stat_level_get(stat_level_barbarian)
		lev_bard = pc.stat_level_get(stat_level_bard)
		lev_figh = pc.stat_level_get(stat_level_fighter)
		lev_cler = pc.stat_level_get(stat_level_cleric)
		lev_drui = pc.stat_level_get(stat_level_druid)
		lev_wiza = pc.stat_level_get(stat_level_wizard)
		lev_sorc = pc.stat_level_get(stat_level_sorcerer)
		lev_monk = pc.stat_level_get(stat_level_monk)
		lev_rang = pc.stat_level_get(stat_level_ranger)
		lev_rogu = pc.stat_level_get(stat_level_rogue)
		lev_pala = pc.stat_level_get(stat_level_paladin)

		hp_min = 6*lev_barb + 5*(lev_figh+lev_pala) + 4*(lev_cler + lev_rogu + lev_drui + lev_monk + lev_rang) + 3*(lev_bard) + 2*(lev_sorc + lev_wiza)
		hp_min = int( hp_min + pc.stat_level_get(stat_level) / 2 )
		if (pc.obj_get_int(obj_f_hp_pts) < hp_min) or (force_av != 0):
			pc.obj_set_int(obj_f_hp_pts, hp_min)

def stat_av():
	# gives the PC "decent" / stereotypical stat scores (Point Buy 32)
	# For now, only "pure" characters 
	# Stats are base stats, not accounting for racial mods
	for pc in game.party:
		lev_barb = pc.stat_level_get(stat_level_barbarian)
		lev_bard = pc.stat_level_get(stat_level_bard)
		lev_figh = pc.stat_level_get(stat_level_fighter)
		lev_cler = pc.stat_level_get(stat_level_cleric)
		lev_drui = pc.stat_level_get(stat_level_druid)
		lev_wiza = pc.stat_level_get(stat_level_wizard)
		lev_sorc = pc.stat_level_get(stat_level_sorcerer)
		lev_monk = pc.stat_level_get(stat_level_monk)
		lev_rang = pc.stat_level_get(stat_level_ranger)
		lev_rogu = pc.stat_level_get(stat_level_rogue)
		lev_pala = pc.stat_level_get(stat_level_paladin)
		
		lev_tot = pc.stat_level_get(stat_level)

		if lev_figh >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 17 + lev_tot/4)
			pc.stat_base_set(stat_dexterity, 14)
			pc.stat_base_set(stat_constitution, 14)
			pc.stat_base_set(stat_intelligence, 13)
			pc.stat_base_set(stat_wisdom, 10)
			pc.stat_base_set(stat_charisma, 8)
			

		if lev_barb >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 18 + lev_tot/4)
			pc.stat_base_set(stat_dexterity, 14)
			pc.stat_base_set(stat_constitution, 16)
			pc.stat_base_set(stat_intelligence, 8)
			pc.stat_base_set(stat_wisdom, 8)
			pc.stat_base_set(stat_charisma, 8)

		if lev_cler >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 14)
			pc.stat_base_set(stat_dexterity, 8)
			pc.stat_base_set(stat_constitution, 12)
			pc.stat_base_set(stat_intelligence, 8)
			pc.stat_base_set(stat_wisdom, 18 + lev_tot/4)
			pc.stat_base_set(stat_charisma, 14)

		if lev_drui >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 14)
			pc.stat_base_set(stat_dexterity, 10)
			pc.stat_base_set(stat_constitution, 14)
			pc.stat_base_set(stat_intelligence, 10)
			pc.stat_base_set(stat_wisdom, 18 + lev_tot/4)
			pc.stat_base_set(stat_charisma, 8)

		if lev_monk >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 16 + lev_tot/4)
			pc.stat_base_set(stat_dexterity, 14)
			pc.stat_base_set(stat_constitution, 12)
			pc.stat_base_set(stat_intelligence, 10)
			pc.stat_base_set(stat_wisdom, 16)
			pc.stat_base_set(stat_charisma, 8)

		if lev_bard >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 10)
			pc.stat_base_set(stat_dexterity, 12)
			pc.stat_base_set(stat_constitution, 10)
			pc.stat_base_set(stat_intelligence, 16)
			pc.stat_base_set(stat_wisdom, 12)
			pc.stat_base_set(stat_charisma, 16 + lev_tot/4)

		if lev_rogu >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 10)
			pc.stat_base_set(stat_dexterity, 16 + lev_tot/4)
			pc.stat_base_set(stat_constitution, 10)
			pc.stat_base_set(stat_intelligence, 16)
			pc.stat_base_set(stat_wisdom, 10)
			pc.stat_base_set(stat_charisma, 14)

		if lev_wiza >= lev_tot*3/4:
			pc.stat_base_set(stat_strength, 10)
			pc.stat_base_set(stat_dexterity, 14)
			pc.stat_base_set(stat_constitution, 14)
			pc.stat_base_set(stat_intelligence, 18 + lev_tot/4)
			pc.stat_base_set(stat_wisdom, 10)
			pc.stat_base_set(stat_charisma, 8)
	return	

def which_class(pc):
	lev_barb = pc.stat_level_get(stat_level_barbarian)
	lev_bard = pc.stat_level_get(stat_level_bard)
	lev_figh = pc.stat_level_get(stat_level_fighter)
	lev_cler = pc.stat_level_get(stat_level_cleric)
	lev_drui = pc.stat_level_get(stat_level_druid)
	lev_wiza = pc.stat_level_get(stat_level_wizard)
	lev_sorc = pc.stat_level_get(stat_level_sorcerer)
	lev_monk = pc.stat_level_get(stat_level_monk)
	lev_rang = pc.stat_level_get(stat_level_ranger)
	lev_rogu = pc.stat_level_get(stat_level_rogue)
	lev_pala = pc.stat_level_get(stat_level_paladin)
	
	lev_tot = pc.stat_level_get(stat_level)
	if (lev_figh >= lev_tot*3/4 and lev_tot > 1) or lev_figh == lev_tot:
		return 'fighter'
	if (lev_barb >= lev_tot*3/4  and lev_tot > 1) or lev_barb == lev_tot:
		return 'barbarian'
	if (lev_cler >= lev_tot*3/4 and lev_tot > 1) or lev_cler == lev_tot:
		return 'cleric'
	if (lev_drui >= lev_tot*3/4 and lev_tot > 1) or lev_drui == lev_tot:
		return 'druid'
	if (lev_monk >= lev_tot*3/4 and lev_tot > 1) or lev_monk == lev_tot:
		return 'monk'
	if (lev_bard >= lev_tot*3/4 and lev_tot > 1) or lev_bard == lev_tot:
		return 'bard'
	if (lev_sorc >= lev_tot*3/4 and lev_tot > 1) or lev_sorc == lev_tot:
		return 'sorcerer'
	if (lev_rogu >= lev_tot*3/4 and lev_tot > 1) or lev_rogu == lev_tot:
		return 'rogue'
	if (lev_wiza >= lev_tot*3/4 and lev_tot > 1) or lev_wiza == lev_tot:
		return 'wizard'
	return 'unknown'


def stat_items(game_stage = 6):
	# Gives stat boosting items

	frostbrand = 0
	for pc in game.party:
		lev_barb = pc.stat_level_get(stat_level_barbarian)
		lev_bard = pc.stat_level_get(stat_level_bard)
		lev_figh = pc.stat_level_get(stat_level_fighter)
		lev_cler = pc.stat_level_get(stat_level_cleric)
		lev_drui = pc.stat_level_get(stat_level_druid)
		lev_wiza = pc.stat_level_get(stat_level_wizard)
		lev_sorc = pc.stat_level_get(stat_level_sorcerer)
		lev_monk = pc.stat_level_get(stat_level_monk)
		lev_rang = pc.stat_level_get(stat_level_ranger)
		lev_rogu = pc.stat_level_get(stat_level_rogue)
		lev_pala = pc.stat_level_get(stat_level_paladin)
		
		lev_tot = pc.stat_level_get(stat_level)

		if which_class(pc) == 'fighter':
			if pc.item_find_by_proto(6244) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6244, pc ) ## Belt of Str +6
			if pc.item_find_by_proto(6201) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6201, pc ) ## Gloves of Dex +6
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6122) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6122, pc ) ## Full Plate +3
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3

		if which_class(pc) == 'barbarian':
			if pc.item_find_by_proto(6244) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6244, pc ) ## Belt of Str +6
			if pc.item_find_by_proto(6201) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6201, pc ) ## Gloves of Dex +6
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6122) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6125, pc ) ## Elven Chain +3
			if frostbrand == 0:
				if pc.item_find_by_proto(4136) == OBJ_HANDLE_NULL:
					create_item_in_inventory( 4136, pc ) ## Frostbrand
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3


		if which_class(pc) == 'cleric':
			if pc.item_find_by_proto(6251) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6251, pc )   ### Amulet of Wis +6
			if pc.item_find_by_proto(6244) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6244, pc ) ## Belt of Str +6
			if pc.item_find_by_proto(6122) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6122, pc ) ## Full Plate +3
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3

		if which_class(pc) == 'druid':
			if pc.item_find_by_proto(6251) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6251, pc )   ### Amulet of Wis +6
			if pc.item_find_by_proto(6244) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6244, pc ) ## Belt of Str +6
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3


		if which_class(pc) == 'monk':
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6244) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6244, pc ) ## Belt of Str +6
			if pc.item_find_by_proto(6201) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6201, pc ) ## Gloves of Dex +6
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3
			if pc.item_find_by_proto(4125) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 4125, pc ) ## Staff of Striking (+3)


		if which_class(pc) == 'bard':
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6254) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6254, pc ) ## Cloak of Cha +6

		if which_class(pc) == 'sorcerer':
			if pc.item_find_by_proto(6254) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6254, pc ) ## Cloak of Cha +6
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3



		if which_class(pc) == 'rogue':
			if pc.item_find_by_proto(6201) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6201, pc ) ## Gloves of Dex +6
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3


		if which_class(pc) == 'wizard':
			if pc.item_find_by_proto(6248) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6248, pc ) ## Headbang of Int +6
			if pc.item_find_by_proto(6242) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6242, pc ) ## Amulet of Health +6
			if pc.item_find_by_proto(6084) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 6084, pc ) ## Ring +3
			if pc.item_find_by_proto(12580) == OBJ_HANDLE_NULL:
				create_item_in_inventory( 12580, pc ) ## Staff of False Life



		pc.money_adj(100 * 500000 - pc.money_get() ) # Set to 500K GP
		pc.identify_all()
		pc.item_wield_best_all()

	return	
	
def giv(pc, proto_id, in_group = 0):	
	if in_group == 0:
		if pc.item_find_by_proto(proto_id) == OBJ_HANDLE_NULL:
			create_item_in_inventory( proto_id, pc )
	else:
		foundit = 0
		for obj in game.party:
			if obj.item_find_by_proto(proto_id) != OBJ_HANDLE_NULL:
				foundit = 1
		if foundit == 0:
			create_item_in_inventory( proto_id, pc )
			return 1
		else:
			return 0
	return

			



def dummy_func(pc):
	if pc == pc:
		if which_class(pc) == 'fighter':
			dumm=1
		if which_class(pc) == 'barbarian':
			dumm=1
		if which_class(pc) == 'cleric':
			dumm=1
		if which_class(pc) == 'druid':
			dumm=1
		if which_class(pc) == 'monk':
			dumm=1
		if which_class(pc) == 'bard':
			dumm=1
		if which_class(pc) == 'sorcerer':
			dumm=1
		if which_class(pc) == 'rogue':
			dumm=1
		if which_class(pc) == 'wizard':
			dumm=1
		
def tenc(): #test encroachment gescheft
	beac = ohn
	for npc in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if npc.name == 14811:
			beac = npc
			break
	countt_encroachers = 1
	countt_all = 1
	for npc in game.obj_list_vicinity(beac.location, OLC_NPC):
		beac.float_mesfile_line( 'mes\\test.mes', countt_all, 1 ) 
		countt_all += 1
		if is_far_from_party(npc, 48) and npc.is_unconscious() == 0:
			attachee.float_mesfile_line( 'mes\\test.mes', countt_encroachers, 2 ) 
			countt_encroachers += 1
			joe = party_closest(npc)
			encroach(npc, joe)

def dsb(radius, ci = 0, hndl=0, nxy=0, cx =0, cy = 0, ec = 'normal'): 
	# detect script bearers
	# floats their object name, description, and san_dying script
	# ec -> extra command
	# ec = 'strat' -> return strategy type, e.g.: type dsb(40, ec='s') in the console to reveal strategy type
	center = game.leader.location
	if ci != 0:
		center = ci
	if cx == 0 or cy == 0:
		cx, cy = location_to_axis(center)
	scriptee_list = []
	for dude in game.obj_list_vicinity(center, OLC_NPC):
		dudex, dudey = location_to_axis(dude.location)
		if (  (dudex - cx)**2 + (dudey - cy)**2  ) <= radius**2:
			scriptee_list.append( (dude.name, dudex, dudey, dude.scripts[12]) )
			if dude.name >= 14000:
				dude.float_mesfile_line( 'mes\\description.mes', dude.name, 1 )
			elif dude.name >= 8000 and dude.name <=9000:
				dude.float_mesfile_line( 'oemes\\oname.mes', dude.name, 1 )
			if str(ec) == 's':
				dude.float_mesfile_line( 'mes\\test.mes', dude.obj_get_int(obj_f_critter_strategy), 1)
			elif str(ec) == '15' or str(ec) == 'san_start_combat' or str(ec) == 'start_combat':
				if dude.scripts[15] != 0: #san_start_combat
					dude.float_mesfile_line( 'mes\\test.mes', dude.scripts[15], 1 )
			elif str(ec) == '19' or str(ec) == 'san_heartbeat' or str(ec) == 'heartbeat':
				if dude.scripts[19] != 0: #san_start_combat
					dude.float_mesfile_line( 'mes\\test.mes', dude.scripts[19], 1 )
			else:
				if dude.scripts[12] != 0: #san_dying
					dude.float_mesfile_line( 'mes\\test.mes', dude.scripts[12], 1 )
	return scriptee_list

def generate_mes_file():
	f = open
	f = open('transaction_sum.mes', 'w')
	f.write("{" + str(0) + "}{Less than 1 GP}\n")
	for pp in range (1,1000):
		f.write("{" + str(pp) + "}{" + str(pp) + " GP}\n")
	for pp in range (0,900):
		f.write("{" + str(1000+ 10*pp) + "}{" + str(1000+ 10*pp) + " GP}\n")
	for pp in range (0,900):
		f.write("{" + str(10000+ 100*pp) + "}{" + str(10000+ 100*pp) + " GP}\n")
	for pp in range (0,900):
		f.write("{" + str(100000+ 1000*pp) + "}{" + str(100000+ 1000*pp) + " GP}\n")
	f.close()


def tgd(hp_desired, radius): 
	# tough guy detector
	# returns a list of critters with HP greater than [hp_desired]
	# list includes the critters "name" , HP, and XY coordinates
	gladius = []
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if moshe.distance_to(game.party[0]) <= radius and moshe.stat_level_get(stat_hp_max) >= hp_desired:
			x,y = lta(moshe.location)
			gladius.append((moshe.name,'hp='+str(moshe.stat_level_get(stat_hp_max)),x,y))
	return gladius

def tai(strat, prot = 14262): # AI tester, optionally select different proto (default - troll)
	game.global_flags[403] = 1 # Test mode flag
	xx, yy = lta(game.leader.location)
	#prot = 14262
	tro = game.obj_create(prot, lfa(xx+3, yy+3) )
	tro.scripts[15] = 3
	tro.obj_set_int(324,strat)
	tro.stat_base_set(stat_hp_max,300) # so he doesn't die from AoOs too quickly
	return tro

def ptai(prot, strat = -999): # AI tester, by proto, optionally alter strategy
	game.global_flags[403] = 1 # Test mode flag
	xx, yy = lta(game.leader.location)
	tro = game.obj_create(prot, lfa(xx+3, yy+3) )
	#tro.scripts[15] = 0
	if strat != -999:
		tro.obj_set_int(324,99+strat)
	tro.stat_base_set(stat_hp_max,300)
	return tro

def subd(party_index):
	# Deal massive nonlethal damage to selected party member
	game.party[party_index].damage(OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new("50d50"))
	return

def kil(party_index):
	# Deal massive LETHAL damage to selected party member
	game.party[party_index].damage(OBJ_HANDLE_NULL, 0 , dice_new("50d50"))
	return


def pron(party_index):
	game.party[party_index].condition_add_with_args( "prone", 0, 0 )
	return


def dex_adj(party_index,new_dex): # adjust base dex
	game.party[party_index].stat_base_set(stat_dexterity,new_dex)
	return

def tsw():	# Test Spiritual Weapon Thingamajig
	game.global_flags[403] = 1 # Test mode flag
	xx, yy = lta(game.leader.location)
	#prot = 14262
	tro = game.obj_create(14262, lfa(xx+3, yy+3) )
	#tro.scripts[15] = 998 # py00998test_combat.py
	tro.obj_set_int(324, 99) # strategy
	tro.stat_base_set(stat_hp_max,300) # so he doesn't die from AoOs too quickly
	return tro


def vrs(var_s, var_e):
	bloke = []
	for snoike in [var_s, var_e+1]:
		bloke.append(game.global_vars[snoike])
	return bloke

def fnl(obj_name,radius):
	gladius = []
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if moshe.distance_to(game.party[0]) <= radius and moshe.name == obj_name:
			gladius.append(moshe)
	return gladius

def bsp(prot):
	a = game.obj_create(prot, game.leader.location)
	a.npc_flag_unset(ONF_KOS)	
	a.move(game.leader.location,0,0)
	return a

def killkos():
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if (moshe.npc_flags_get() & ONF_KOS != 0 and moshe.npc_flags_get() & ONF_KOS_OVERRIDE == 0 and moshe.scripts[22] == 0 and moshe.leader_get() == ohn):
			# moshe.critter_kill_by_effect()
			damage_dice = dice_new( '50d50' )
			moshe.damage( game.party[0], 0, damage_dice )

	return

def kf():
	# Kill foes
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		hostile = 0
		for pc in game.party:
			if moshe.reaction_get( pc ) <= 0:
				hostile = 1
		if (hostile ==  1 and moshe.leader_get() == ohn):
			# moshe.critter_kill_by_effect()
			damage_dice = dice_new( '50d50' )
			moshe.damage( game.party[0], 0, damage_dice )

def kuf( c_name = -1):
	# Kill unfriendlies
	# c_name - of particular name
	if type(c_name) == type(1):
		for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
			if (moshe.reaction_get(game.party[0]) <= 0 or moshe.is_friendly(game.party[0]) == 0) and ( not (moshe.leader_get() in game.party) and moshe.object_flags_get() & OF_DONTDRAW == 0) and (moshe.name == c_name or c_name == -1):
				# moshe.critter_kill_by_effect()
				damage_dice = dice_new( '50d50' )
				moshe.damage( game.party[0], 0, damage_dice )
	elif type(c_name) == type('asdf'):
		for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
			if (moshe.reaction_get(game.party[0]) <= 0 or moshe.is_friendly(game.party[0]) == 0) and ( not (moshe.leader_get() in game.party) and moshe.object_flags_get() & OF_DONTDRAW == 0) and ( (str(moshe).lower().find(c_name.lower()) != -1) ):
				# moshe.critter_kill_by_effect()
				damage_dice = dice_new( '50d50' )
				moshe.damage( game.party[0], 0, damage_dice )


def vlist3(radius):
	gladius = []
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if moshe.distance_to(game.party[0]) <= radius:
			gladius.append(moshe)
	return gladius

def nxy(radius):
	gladius = []
	for moshe in game.obj_list_vicinity(game.leader.location, OLC_NPC):
		if moshe.distance_to(game.party[0]) <= radius:
			x,y = lta(moshe.location)
			gladius.append((moshe.name,x,y))
	return gladius


def hl(obj):
	#highlight the sucker
	game.particles('ef-minocloud', obj)
	return

def make_slow():
	for pc in game.party:
		pc.stat_base_set(stat_dexterity,4)
	return



def fl(flag_no):
	if game.global_flags[flag_no] == 0:
		game.global_flags[flag_no] = 1
	return game.global_flags[flag_no]

def vr(var_no):
	return game.global_vars[var_no]


def tyme():
	return ('month = ' + str(game.time.time_game_in_months(game.time)), 'day = '+ str(game.time.time_game_in_days(game.time)), 'hour = ' + str(game.time.time_game_in_hours(game.time)), 'minute = ' + str(game.time.time_game_in_minutes(game.time)) )


def gp( name):
	a = fnn( name )
	if a != OBJ_HANDLE_NULL:
		return a.obj_get_int(obj_f_npc_pad_i_5)
	else:
		return OBJ_HANDLE_NULL

def sp(name, num):
	a = fnn( name )
	if a != OBJ_HANDLE_NULL:
		a.obj_set_int(obj_f_npc_pad_i_5, num)
	else:
		return OBJ_HANDLE_NULL

def dof(obj): 
	## Display Object Flags
	## A handy little function to display an objects' object flags
	col_size = 10
	object_flags_list = ['OF_DESTROYED','OF_OFF','OF_FLAT','OF_TEXT','OF_SEE_THROUGH','OF_SHOOT_THROUGH','OF_TRANSLUCENT','OF_SHRUNK','OF_DONTDRAW','OF_INVISIBLE','OF_NO_BLOCK','OF_CLICK_THROUGH','OF_INVENTORY','OF_DYNAMIC','OF_PROVIDES_COVER','OF_RANDOM_SIZE','OF_NOHEIGHT','OF_WADING','OF_UNUSED_40000','OF_STONED','OF_DONTLIGHT','OF_TEXT_FLOATER','OF_INVULNERABLE','OF_EXTINCT','OF_TRAP_PC','OF_TRAP_SPOTTED','OF_DISALLOW_WADING','OF_UNUSED_0800000','OF_HEIGHT_SET','OF_ANIMATED_DEAD','OF_TELEPORTED','OF_RADIUS_SET']
	lista = []
	for p in range(0, 31):
		lista.append(''.join([ object_flags_list[p],' - ',str(obj.object_flags_get() & pow(2,p) != 0) ]))
	lista.append(''.join([ object_flags_list[31],' - ',str(obj.object_flags_get() & OF_RADIUS_SET != 0) ]))
	lenmax = 1
	for p in range(0,31):
		if len(lista[p]) > lenmax:
			lenmax = len(lista[p])
	##print 'lenmax = ',str(lenmax)
	print ''
	for p in range(0,col_size+1):
		len1 = len(''.join([ object_flags_list[p],' - ',str(obj.object_flags_get() & pow(2,p) != 0) ]))
		len2 = len(''.join([ object_flags_list[p+col_size+1],' - ',str(obj.object_flags_get() & pow(2,p+col_size+1) != 0) ]))
		if p >= col_size-1:
			hau = OF_RADIUS_SET
		else:
			hau = pow(2,p+2*col_size+2)
		har1 = ''
		har2 = ''
		for p1 in range(0, lenmax-len1+1):
			har1 += '  '
		for p2 in range(0, lenmax-len2+1):
			har2 += '   '
		if p < col_size:
			print ''.join([ object_flags_list[p],' - ',str(obj.object_flags_get() & pow(2,p) != 0) ]),har1,''.join([ object_flags_list[p+col_size+1],' - ',str(obj.object_flags_get() & pow(2,p+col_size+1) != 0) ]),har2,''.join([ object_flags_list[p+2*col_size+2],' - ',str(obj.object_flags_get() & hau != 0) ])
		else:
			print ''.join([ object_flags_list[p],' - ',str(obj.object_flags_get() & pow(2,p) != 0) ]),har1,''.join([ object_flags_list[p+col_size+1],' - ',str(obj.object_flags_get() & pow(2,p+col_size+1) != 0) ])


	return

def dnf(obj): 
	## A handy little function to display an objects' NPC flags
	col_size = 10
	npc_flags_list = ['ONF_EX_FOLLOWER','ONF_WAYPOINTS_DAY','ONF_WAYPOINTS_NIGHT','ONF_AI_WAIT_HERE','ONF_AI_SPREAD_OUT','ONF_JILTED','ONF_LOGBOOK_IGNORES','ONF_UNUSED_00000080','ONF_KOS','ONF_USE_ALERTPOINTS','ONF_FORCED_FOLLOWER','ONF_KOS_OVERRIDE','ONF_WANDERS','ONF_WANDERS_IN_DARK','ONF_FENCE','ONF_FAMILIAR','ONF_CHECK_LEADER','ONF_NO_EQUIP','ONF_CAST_HIGHEST','ONF_GENERATOR','ONF_GENERATED','ONF_GENERATOR_RATE1','ONF_GENERATOR_RATE2','ONF_GENERATOR_RATE3','ONF_DEMAINTAIN_SPELLS','ONF_UNUSED_02000000','ONF_UNUSED_04000000','ONF_UNUSED08000000','ONF_BACKING_OFF','ONF_NO_ATTACK','ONF_BOSS_MONSTER','ONF_EXTRAPLANAR']
	lista = []
	for p in range(0, 31):
		lista.append(''.join([ npc_flags_list[p],' - ',str(obj.npc_flags_get() & pow(2,p) != 0) ]))
	lista.append(''.join([ npc_flags_list[31],' - ',str(obj.npc_flags_get() & OF_RADIUS_SET != 0) ]))
	lenmax = 1
	for p in range(0,31):
		if len(lista[p]) > lenmax:
			lenmax = len(lista[p])
	##print 'lenmax = ',str(lenmax)
	print ''
	for p in range(0,col_size+1):
		len1 = len(''.join([ npc_flags_list[p],' - ',str(obj.npc_flags_get() & pow(2,p) != 0) ]))
		len2 = len(''.join([ npc_flags_list[p+col_size+1],' - ',str(obj.npc_flags_get() & pow(2,p+col_size+1) != 0) ]))
		if p >= col_size-1:
			hau = ONF_EXTRAPLANAR
		else:
			hau = pow(2,p+2*col_size+2)
		har1 = ''
		har2 = ''
		for p1 in range(0, lenmax-len1+1):
			har1 += '  '
		for p2 in range(0, lenmax-len2+1):
			har2 += '   '
		if p < col_size:
			print ''.join([ npc_flags_list[p],' - ',str(obj.npc_flags_get() & pow(2,p) != 0) ]),har1,''.join([ npc_flags_list[p+col_size+1],' - ',str(obj.npc_flags_get() & pow(2,p+col_size+1) != 0) ]),har2,''.join([ npc_flags_list[p+2*col_size+2],' - ',str(obj.npc_flags_get() & hau != 0) ])
		else:
			print ''.join([ npc_flags_list[p],' - ',str(obj.npc_flags_get() & pow(2,p) != 0) ]),har1,''.join([ npc_flags_list[p+col_size+1],' - ',str(obj.npc_flags_get() & pow(2,p+col_size+1) != 0) ])


	return


def te():
	#test earth temple stuff
	uberize()
	gimme(give_earth = 1)
	earthaltar()
	return

def ta():
	#test air temple stuff
	game.global_flags[108] = 1 #makes water bugbears defect
	uberize()
	gimme(give_air = 1)
	airaltar()
	return

def tw():
	#test water temple stuff
	uberize()
	gimme(give_water = 1)
	belsornig()
	return



def t():
	uberizeminor()
	gimme()
	hommlet()
	return



def portraits_verify():
	# this assumes portraits are laid out in {number} {file} format
	# Otherwise the script can get borked
	
	# Init
	s = 'initial value'
	portrait_index = -1
	previous_portrait = -1
	found_error_flag = 0
	ff = open('modules\\ToEE\\portrait_checking_result.txt','w')
	i_file = open('portraits.mes','r')
	
	while s !='':
		s = i_file.readline()
		s2 = s.split('{')
		
		if len(s2) == 3: # check if it's an actual portrait line with {number} {file} format - will return an array ['','number} ','file}'] entry
			s3 = s2[1].replace("}","").strip()
			if s3.isdigit():
				if portrait_index == -1:
					portrait_index = int(s3)
					previous_portrait = portrait_index
				else:
					portrait_index = int(s3)
					# checks:
					# 1. portrait in the same decimal range are not sequential
					# 2. portrait '0' is identical to previous group's 2nd decimal
					if (  (not (portrait_index - previous_portrait == 1) ) and ( portrait_index % 10 != 0 )   ):
						ff.write( 'Error! Portrait number ' + str(portrait_index) + ' is not in sequence.' + '\n')
						print 'Error! Portrait number ' + str(portrait_index) + ' is not in sequence.' + '\n'
						found_error_flag = 1
					elif (  portrait_index % 10 == 0 and ( portrait_index - (portrait_index%10) ) == ( previous_portrait - (previous_portrait%10) ) and previous_portrait != -1 ):
						ff.write( 'Error! Portrait number ' + str(portrait_index) + ' is duplicate of previous group.' + '\n')
						print 'Error! Portrait number ' + str(portrait_index) + ' is duplicate of previous group.' + '\n'
						found_error_flag = 1
					previous_portrait = portrait_index
			else:
				ff.write( 'Error! bracket with a non-number within! ' + s3 +'\n')
				print 'Error! bracket with a non-number within! ' + s3 +'\n'
				found_error_flag = 1
	if not found_error_flag:
		ff.write( 'Portraits file examined - no errors found.')
		print 'Portraits file examined - no errors found.'
	ff.close()
	i_file.close()


#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————

def uberizeminor():
	for pc in game.party:
		pc.stat_base_set(stat_strength,24)
		pc.stat_base_set(stat_hp_max, 70)
		pc.stat_base_set(stat_constitution,24)
		pc.stat_base_set(stat_charisma,24)
		pc.stat_base_set(stat_dexterity,24)
		pc.stat_base_set(stat_wisdom,24)
		if pc.stat_level_get(stat_level_wizard)>0 or pc.stat_level_get(stat_level_rogue)>0 or pc.stat_level_get(stat_level_bard)>0:
			pc.stat_base_set(stat_intelligence,18)
	return

def uberize():
	for pc in game.party:
		pc.stat_base_set(stat_strength,38)
		pc.stat_base_set(stat_hp_max, 300)
		pc.stat_base_set(stat_constitution,45)
		pc.stat_base_set(stat_charisma,100)
		pc.stat_base_set(stat_dexterity,41)
		pc.stat_base_set(stat_wisdom,100)
		if pc.stat_level_get(stat_level_wizard)>0 or pc.stat_level_get(stat_level_rogue)>0 or pc.stat_level_get(stat_level_bard)>0:
			pc.stat_base_set(stat_intelligence,18)
	return

def uberizemajor():
	for pc in game.party:
		pc.stat_base_set(stat_strength,210)
		pc.stat_base_set(stat_hp_max, 300)
		pc.stat_base_set(stat_constitution,45)
		pc.stat_base_set(stat_charisma,100)
		pc.stat_base_set(stat_dexterity,100)
		pc.stat_base_set(stat_wisdom,100)
		if pc.stat_level_get(stat_level_wizard)>0 or pc.stat_level_get(stat_level_rogue)>0 or pc.stat_level_get(stat_level_bard)>0:
			pc.stat_base_set(stat_intelligence,18)
	return




#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————





def gimme_minor():
	for pc in game.party:
		if pc.item_find_by_proto(6105) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 6015, pc )
		#if pc.item_find_by_proto(6109) == OBJ_HANDLE_NULL:
		#	create_item_in_inventory( 6109, pc )
		#if pc.item_find_by_proto(6110) == OBJ_HANDLE_NULL:
		#	create_item_in_inventory( 6110, pc )
		#if pc.item_find_by_proto(6111) == OBJ_HANDLE_NULL:
		#	create_item_in_inventory( 6111, pc )
		#if pc.item_find_by_proto(6112) == OBJ_HANDLE_NULL:
		#	create_item_in_inventory( 6112, pc )
		if pc.item_find_by_proto(6113) == OBJ_HANDLE_NULL: #greater temple robes
			create_item_in_inventory( 6113, pc )
## the above are temple robes and eye of flame cloak
		if pc.item_find_by_proto(12262) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 12262, pc )
		create_item_in_inventory( 6266, pc ) #amulet AC +5
		create_item_in_inventory( 6101, pc ) #ringa Fire Res 15
		create_item_in_inventory( 6115, pc ) #bracers +5
		create_item_in_inventory( 5013, pc ) #bolts +3
		create_item_in_inventory( 4085, pc ) #sword +5
		studded_leather = 0
		if pc.item_find_by_proto(6056) != OBJ_HANDLE_NULL:
			pc.item_find_by_proto(6056).destroy()
			studded_leather = 1

		pc.item_wield_best_all()

		if studded_leather == 1:
			create_item_in_inventory(6056, pc)
		if pc.item_find_by_proto(4177) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 4177, pc ) 
## masterwork light xbow
		pc.identify_all()


def gimme(gversion = 5, give_earth = 0, give_air = 0, give_water = 0):
	# Vanilla version has no wand of fireball (9th), so use gimme(1) for vanilla!
	for pc in game.party:
		if pc.item_find_by_proto(6105) == OBJ_HANDLE_NULL and pc.item_worn_at(10).name != 3005: # Eye of Flame Cloak
			create_item_in_inventory( 6015, pc )

		if pc.item_find_by_proto(6109) == OBJ_HANDLE_NULL and give_air == 1:
			create_item_in_inventory( 6109, pc ) ## Air Temple robes
		if pc.item_find_by_proto(6110) == OBJ_HANDLE_NULL and give_earth == 1:
			create_item_in_inventory( 6110, pc ) ## Earth Temple robes
		#if pc.item_find_by_proto(6111) == OBJ_HANDLE_NULL:
		#	create_item_in_inventory( 6111, pc )
		if pc.item_find_by_proto(6112) == OBJ_HANDLE_NULL and give_water == 1:
			create_item_in_inventory( 6112, pc ) ## Water Temple robes

		if pc.item_find_by_proto(6113) == OBJ_HANDLE_NULL: # Greater Temple robes
			create_item_in_inventory( 6113, pc )

		if pc.item_find_by_proto(12262) == OBJ_HANDLE_NULL: # Wand of knock
			create_item_in_inventory( 12262, pc )
		if gversion >= 5:
			if pc.item_find_by_proto(12619) == OBJ_HANDLE_NULL: 
				create_item_in_inventory( 12619, pc ) # Wand of Fireball (9th)

		if pc.item_find_by_proto(6266) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 6266, pc ) # Amulet AC +5
		if pc.item_find_by_proto(6101) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 6101, pc ) # Ring of Fire Res 15
		if pc.item_find_by_proto(6115) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 6115, pc ) # Bracers +5
		if pc.item_find_by_proto(4219) == OBJ_HANDLE_NULL and pc.item_find_by_proto(4118) == OBJ_HANDLE_NULL:
			if ( pc.stat_level_get(stat_alignment) & ALIGNMENT_EVIL == 0 ):
				create_item_in_inventory( 4219, pc ) # Holy Ranseur + 1
			else:
				create_item_in_inventory( 4118, pc ) # Glaive
		create_item_in_inventory( 5013, pc ) # Bolts +3 x 10
		if pc.item_find_by_proto(4085) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 4085, pc ) # Longsword +5
## weapons and armor: xbow, holy ranseur

		studded_leather = 0
		if pc.item_find_by_proto(6056) != OBJ_HANDLE_NULL:
			pc.item_find_by_proto(6056).destroy()
			studded_leather = 1

		pc.item_wield_best_all()

		if studded_leather == 1:
			create_item_in_inventory(6056, pc)
		if pc.item_find_by_proto(4177) == OBJ_HANDLE_NULL:
			create_item_in_inventory( 4177, pc ) 
## masterwork light xbow
		pc.identify_all()

	return





#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————


def s(prot):
	(x,y) = lta(game.leader.location)
	a = spawn(prot,x,y+1)
	return a


def det():
	return (game.party[0].map, loc())




def tp(map,X,Y):
	game.fade_and_teleport(0,0,0,map,X,Y)
	return

def loc():
	return location_to_axis(game.party[0].location)

def lfa( x, y ):
	# initialize loc to be a LONG integer
	loc = 0L + y
	loc = ( loc << 32 ) + x
	return loc

def lta( loc ):
	y = loc >> 32
	x = loc & 4294967295
	return ( x, y )

def lvl():  # CB - sets entire groups experience points to xp
	pc = game.leader
	for obj in pc.group_list():
		curxp = obj.stat_level_get(stat_experience)
		newxp = curxp + 600000
		obj.stat_base_set(stat_experience, newxp)
	return 	1

def fnn( name = -1, living_only = 1, multiple = 0 ):
	for npc in game.obj_list_vicinity( game.leader.location, OLC_NPC ):
		if type(name) == type(1):
			if (npc.name == name and multiple == 0):
				return npc
			elif name == -1 | multiple != 0:
				if living_only == 1 and npc.is_unconscious() == 1:
					continue
				if npc.name != name and multiple != 0:
					continue
				xx,yy = lta(npc.location)
				print str(npc) + ',      name ID = ' + str(npc.name) + ',    x = ' + str(xx) + ',    y = ' + str(yy)
				npc.float_mesfile_line( 'mes\\test.mes', int(xx), 1 ) 
				npc.float_mesfile_line( 'mes\\test.mes', int(yy), 1 ) 
		elif type(name) == type('asdf'):
			if (str(npc).lower().find(name.lower()) != -1):
				return npc


	return OBJ_HANDLE_NULL

	
def fpn(name = -1, living_only = 1, multiple = 0):  ## find PCs near
	for pc in game.obj_list_vicinity( game.leader.location, OLC_PC ):
		if type(name) == type(1):
			if (pc.name == name and multiple == 0):
				return pc
			elif name == -1 | multiple != 0:
				if living_only == 1 and pc.is_unconscious() == 1:
					continue
				if pc.name != name and multiple != 0:
					continue
				xx,yy = lta(pc.location)
				print str(pc) + ',      name ID = ' + str(pc.name) + ',    x = ' + str(xx) + ',    y = ' + str(yy)
				pc.float_mesfile_line( 'mes\\test.mes', int(xx), 1 ) 
				pc.float_mesfile_line( 'mes\\test.mes', int(yy), 1 ) 
		elif type(name) == type('asdf'):
			if (str(pc).lower().find(name.lower()) != -1):
				return pc
	return OBJ_HANDLE_NULL	
	
def tpca(): # test adding PC via follower_add()
	a = fpn('va') # vadania
	game.leader.follower_add(a)
	
	
def vlist( npc_name = -1):
	moshe = game.obj_list_vicinity(game.leader.location, OLC_NPC)
	if npc_name != -1:
		return_list = []
		for obj in moshe:
			if type(npc_name) == type(1):
				if npc_name == obj.name:
					return_list.append(obj)
			elif type(npc_name) == type( [1, 2]):
				if obj.name in npc_name:
					return_list.append(obj)
		return return_list
	else:
		return moshe

def vlist2():
	# looks for nearby containers
	moshe = game.obj_list_vicinity(game.leader.location, OLC_CONTAINER)
	return moshe


def spawn(prot,x,y):
	moshe = game.obj_create(prot,lfa(x,y))
	if (moshe != OBJ_HANDLE_NULL):
		return moshe
	return OBJ_HANDLE_NULL

def alldie():
	for obj in game.obj_list_vicinity( game.party[0].location, OLC_CRITTERS ):
		if obj not in game.party[0].group_list() and obj.name != 14455:
			obj.critter_kill_by_effect()
			# damage_dice = dice_new( '104d20' )
			# obj.damage( OBJ_HANDLE_NULL, 0, damage_dice )
	return 1











#——————————————————————————————————————————————————————————————————————————————————————————
#	@	@	@	@	@	@	@	@	@	@	
#——————————————————————————————————————————————————————————————————————————————————————————











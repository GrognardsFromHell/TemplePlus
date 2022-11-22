from toee import *
from utilities import *
from py00439script_daemon import record_time_stamp, get_v, set_v, get_f, set_f, npc_set, npc_unset, npc_get, tsc, tpsts, within_rect_by_corners
# from Livonya import break_free

def break_free(critter, range):
	return 0

def ProtectTheInnocent( attachee, triggerer):
	return # Fuck this script
	print "ProtectTheInnocent, Attachee: " + str(attachee) + " Triggerer: " + str(triggerer)
	handleList = {
	8000: 'attack',  #elmo
	8001: 'runoff', #paida 
	8014: 'attack', #otis
	8015: 'attack', #meleny 
	8021: 'attack', #ydey 
	8022: 'attack', #murfles
	8031: 'attack', #thrommel 
	8039: 'attack',#taki 
	8054: 'attack',#burne 
	8060: 'attack',#morgan 
	8069: 'attack', #pishela
	8071: 'attack', #rufus
	8072: 'attack', #spugnoir
	8714: 'attack', #holly
	8730: 'attack', #ronald
	8731: 'attack' #ariakas
	}
	
	specialCases = { # placeholder for now
	8014: 0 #otis]
	}
	if (triggerer.type == obj_t_pc):
		for f,p in handleList.items():
			if anyone( triggerer.group_list(), "has_follower", f):
				dude = find_npc_near(triggerer, f)
				if (dude != OBJ_HANDLE_NULL):
					if (attachee.name == 8014 and dude.name == 8000): # otis & elmo
						continue
					else:
						triggerer.follower_remove(dude)
						dude.float_line( 20000, triggerer)
						if (p == 'attack'):
							dude.attack(triggerer)
						else:
							dude.runoff(dude.location-3)


def should_modify_CR( attachee ):
	return 0 # now done in the DLL properly!!! MWAHAHAHA
	# uses npc_get flag # 31
	#party_av_level = get_av_level()
	#if party_av_level > 10 and npc_get(attachee, 31) == 0:
	#	return 1
	#else:
	#	return 0

	
def get_av_level():
	# calculates average level of top 50% of the party
	# (rounded down; for odd-sized parties, the middle is included in the top 50%)

	# record every party member's level
	level_array = []
	for qq in game.party:
		level_array.append(qq.stat_level_get(stat_level))
	# sort
	level_array.sort()
	
	# calculate average of the top 50%
	level_sum = 0
	rr = range( len(level_array)/2 , len(level_array) )
	for qq in rr:
		level_sum = level_sum + level_array[qq]

	party_av_level = level_sum / len(rr)
		
	return party_av_level
	
		

def CR_tot_new( party_av_level, CR_tot ):
	# functions returns the desired total CR (to used for calculating new obj_f_npc_challenge_rating)
	#   such that parties with CL > 10 will get a more appropriate XP reward
	# party_av_level - the average CL to be simulated
	# CR_tot - the pre-adjusted total CR (natural CR + CL); 
	#    e.g. Rogue 15 with -2 CR mod -> CR_tot = 13; the -2 CR mod will (probably) get further adjusted by this function
	
	expected_xp = calc_xp_proper(party_av_level, CR_tot)
	
	best_CR_fit = CR_tot
	
	for qq in range(CR_tot-1, min(5, CR_tot-2) , -1):
		if abs( calc_xp_proper(10, qq) - expected_xp) < abs( calc_xp_proper(10, best_CR_fit) - expected_xp) and abs( calc_xp_proper(10, qq) - expected_xp) < abs( calc_xp_proper(10, CR_tot) - expected_xp):
			best_CR_fit = qq

	return best_CR_fit


		
def CR_mod_new( attachee, party_av_level = -1 ):
	if party_av_level == -1:
		party_av_level = get_av_level()
	CR_tot = attachee.stat_level_get(stat_level) + attachee.obj_get_int(obj_f_npc_challenge_rating)
	return ( CR_tot_new(party_av_level, CR_tot) - attachee.stat_level_get(stat_level) )

def modify_CR( attachee, party_av_level = -1  ):
	npc_set( attachee, 31 )
	if party_av_level == -1:
		party_av_level = get_av_level()
	attachee.obj_set_int(obj_f_npc_challenge_rating, CR_mod_new( attachee, party_av_level ) )


def calc_xp_proper( party_av_level, CR_tot ):
	# returns expected XP award
	xp_gain = party_av_level * 300
	xp_mult = 2**long(  abs(CR_tot - party_av_level) / 2) 
	
	if (CR_tot - party_av_level) % 2 == 1:
		xp_mult = xp_mult * 1.5
		
	if party_av_level > CR_tot:
		return long(xp_gain / xp_mult)
	else:
		return long(xp_gain * xp_mult)
	

def create_break_free_potion( attachee ):
	## creates a potion of breakfree (the workaround for making AI attempt break free)
	## ALSO, THIS IS USED FOR BREAK FREE
	while(attachee.item_find(8903) != OBJ_HANDLE_NULL):
		attachee.item_find(8903).destroy()
	#if (attachee.d20_query(Q_Is_BreakFree_Possible)): # workaround no longer necessary !
	#	create_item_in_inventory( 8903, attachee )
	##	attachee.d20_send_signal(S_BreakFree)
	
	
def Spiritual_Weapon_Begone( attachee ):
	# mwahaha this is no longer necessary!!!
	pass
	return		
			
	
		
def timed_restore_state( attachee, closest_jones, orig_strat ):
	if closest_jones.obj_get_int(obj_f_hp_damage) >= 1000:
		closest_jones.obj_set_int(obj_f_hp_damage, closest_jones.obj_get_int(obj_f_hp_damage) - 1000 )
		closest_jones.stat_base_set(stat_hp_max, closest_jones.stat_base_get(stat_hp_max) - 1000 )

		attachee.obj_set_int(obj_f_critter_strategy, orig_strat)

from toee import *
from __main__ import game
from utilities import *
from Co8 import *
#from itt import *
from array import *
from math import atan2
from py00648script_daemon import *
from t import *




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
	if (lev_pala >= lev_tot*3/4 and lev_tot > 1) or lev_pala == lev_tot:
		return 'paladin'
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



def quickstart(simulated_game_state = 0, cheat_items = 1, autokill_on = 1):
	#gearup(simulated_game_state, cheat_items = 1)
	san_dying(game.leader, game.leader) # assign the script daemon to someone
	print str(simulated_game_state)

	if simulated_game_state >= 0: # Beginning of Keep stuff
		game.global_flags[26] = 1  # Players have gone into Keep
		game.quests[21].state = qs_accepted # Silk Road - silk for Flay
		# Noticeboard quests:
		game.areas[9] = 1 # Swamps
		game.quests[10].state = qs_accepted  # Find missing adventurers quest
		game.areas[2] = 1 # Spider woods
		game.quests[17].state = qs_accepted # Spider problem quest
		game.global_flags[10] = 1 # Found out about Chapel of Kord
		game.global_flags[85] = 1 # Met Mannes
		game.global_vars[4] = 3 # Permanent inn lodging
		if game.leader.money_get() >= 5000:
			game.leader.money_adj(-5000)
		
		# Stuff that depends on skill checks: (often Gather Info)
		game.areas[5] = 1 # Northern woods
		game.quests[19].state = qs_accepted # Kepler's clear harpies quest

		
		game.global_vars[3] = 12  # Envenomed blade investigation - questioned Nobby
		game.global_vars[2] = 1 # Ricario (Apothecary) mentioned poison
		game.quests[2].state = qs_accepted  # Cauldron Quest for Apothecary
		
		game.global_flags[29] = 1  # Told about Cauldron incident
		
		game.global_flags[66] = 1 # have learned about Priest
		game.quests[1].state = qs_accepted # Get Provisioner's Wife into the Guild
		

	if simulated_game_state >= 1:
		print 'Executing Northern Oak Woods, Lizard Swamp, Spider Woods...'
		# Having just done the early Keep phase, going on the Swamps, Harpy Woods and Spider Woods

		if autokill_on == 1:
			set_f('qs_autokill_oak_woods')
			set_f('qs_autokill_lizard_swamp')
			set_f('qs_autokill_spider_woods')
		################
		# Kill section #
		################


	if simulated_game_state >= 2:
		if autokill_on == 1:
			set_f('qs_autokill_moathouse', 1)
		# Having just completed Moathouse + Emridy + Welkwood Bog
		#for pc in game.party[0].group_list():
		#	if pc.stat_level_get(stat_experience) <= 6000:
		#		pc.stat_base_set(stat_experience, 6000)
		game.quests[24].state = qs_accepted # Dragon Quest
		game.areas[8] = 1 # Dragon Swamp

		
		
		game.global_flags[38] = 1 # Spider Queen won't relent flag
		
		game.story_state = 2
		game.areas[8] = 1 # Moathouse Cave Exit
		game.areas[2] = 1 # Deklo
		print 'Executing Moathouse + Emridy Meadows...'

	if simulated_game_state >= 3:
		# Having Finished Nulb + HB 
		# I.E. auto-kill Nulb and HB
		# preparing for "legitimate" AoH + Revenge Encounter + Moathouse Respawn ( + Temple )
		#for pc in game.party[0].group_list():
		#	if pc.stat_level_get(stat_experience) <= 16000:
		#		pc.stat_base_set(stat_experience, 16000)		
		print 'Executing Nulb, HB'
		game.story_state = 3
		game.areas[3] = 1 # Nulb
		game.areas[6] = 1 # Imeryds
		game.areas[4] = 1 # HB
		game.quests[35].state = qs_accepted # Grud's story
		game.quests[41].state = qs_accepted # Preston's tooth ache
		game.quests[42].state = qs_accepted # Assassinate Lodriss
		game.quests[59].state = qs_accepted # Free Serena

		game.quests[60].state = qs_accepted # Mona's Orb
		game.quests[63].state = qs_accepted # Bribery for justice
		if autokill_on == 1:
			set_f('qs_autokill_nulb', 1)



	if simulated_game_state >= 3.5:
		game.quests[65].state = qs_accepted # Hero's Prize Quest
		game.global_vars[972] = 2 # Have talked to Kent about Witch
		set_f('qs_arena_of_heroes_enable')




	if simulated_game_state >= 4:
		# Autokill Temple, AoH, Revenge Encounter, MR
		print 'Executing Temple, AoH, Moathouse Respawn, Revenge Encounter'
		if autokill_on == 1:
			set_f('qs_autokill_temple')
		game.story_state = 4
		game.areas[5] = 1 # Temple
		game.quests[65].state = qs_accepted # Hero's Prize Quest
		game.global_flags[944] = 1

	if simulated_game_state >= 5:
		# Autokill Greater Temple, Verbobonc (minus slavers)
		print 'Executing Greater Temple, Verbobonc'
		if autokill_on == 1:
			set_f('qs_autokill_greater_temple')
		game.story_state = 5
		game.areas[11] = 1 # Temple Burnt Farmhouse
		game.areas[14] = 1 # Verbobonc

	if simulated_game_state >= 6:
		print 'Executing Nodes, WotGS'
		if autokill_on == 1:
			set_f('qs_autokill_nodes')
		game.story_state = 6




def gearup(o_ride = -1, cheat_items = 1):
	s_rogue_items = []
	s_tank_weapons_2 = []
	s_tank_armor_2 = []

	figh_pc = OBJ_HANDLE_NULL
	barb_pc = OBJ_HANDLE_NULL
	bard_pc = OBJ_HANDLE_NULL
	rogu_pc = OBJ_HANDLE_NULL
	cler_pc = OBJ_HANDLE_NULL
	drui_pc = OBJ_HANDLE_NULL
	monk_pc = OBJ_HANDLE_NULL
	sorc_pc = OBJ_HANDLE_NULL
	wiza_pc = OBJ_HANDLE_NULL
	pala_pc = OBJ_HANDLE_NULL
	
	for pc in game.party:
		if which_class(pc) == 'fighter':
			figh_pc = pc
		if which_class(pc) == 'paladin':
			pala_pc = pc
		if which_class(pc) == 'barbarian':
			barb_pc = pc
		if which_class(pc) == 'bard':
			bard_pc = pc
		if which_class(pc) == 'rogue':
			rogu_pc = pc
		if which_class(pc) == 'cleric':
			cler_pc = pc
		if which_class(pc) == 'druid':
			drui_pc = pc
		if which_class(pc) == 'monk':
			monk_pc = pc
		if which_class(pc) == 'sorcerer':
			sorc_pc = pc
		if which_class(pc) == 'wizard':
			wiza_pc = pc
		brown_farmer_garb = pc.item_find_by_proto(6142)
		if brown_farmer_garb != OBJ_HANDLE_NULL:
			brown_farmer_garb.destroy()

	if game.story_state ==2:
		dummy = 1


	for pc in game.party:
		if game.story_state <= 1 or pc.map == 5107 or o_ride == 0:
			if which_class(pc) == 'fighter':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4010) # Greatsword
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6093) # Chain Shirt
			if which_class(pc) == 'paladin':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4010) # Greatsword
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6093) # Chain Shirt
			if which_class(pc) == 'barbarian':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4064) # Greataxe
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6055) # Barbarian Armor

			if which_class(pc) == 'cleric':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4070) # Morningstar
				giv(pc, 6070) # Large Wooden Shield
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6153) # Fine Scalemail
				if cheat_items == 1:
					giv(pc, 12231) # Wand of Holy Smite
					giv(pc, 12178) # Wand of Flame Strike

			if which_class(pc) == 'druid':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4045) # Scimitar
				giv(pc, 6070) # Large Wooden Shield
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6009) # Druid Hide
				if cheat_items == 1:
					giv(pc, 12178) # Wand of Flame Strike


			if which_class(pc) == 'monk':
				giv(pc, 4243) # Quarterstaff

			if which_class(pc) == 'bard':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6013) # Brown Leather Armor
				giv(pc, 12564) # Mandolin
				giv(pc, 12677, 1) # Spy

			if which_class(pc) == 'sorcerer':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff
				if cheat_items == 1:
					giv(pc, 12620) # Wand of Fireball (10th)
					giv(pc, 12262) # Wand of Knock

			if which_class(pc) == 'rogue':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				# giv(pc, 6031) # Eyeglasses
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6042) # Black Leather Armor
				giv(pc, 12012) # Thieves Tools
				giv(pc, 12767) # Lockslip Grease
				giv(pc, 12677, 1) # Spyglass

			if which_class(pc) == 'wizard':
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6151) # Red Mystic Garb
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff
				giv(pc, 9229) # Grease Scroll
				giv(pc, 12848) # Scholar's kit
				if cheat_items == 1:
					giv(pc, 12620) # Wand of Fireball (10th)
					giv(pc, 12262) # Wand of Knock




		if game.story_state == 2 or o_ride == 2: 
			# End of Moathouse - Tackling Nulb and HB
			# Scrolls: Stinking Cloud, Knock, Ray of Enfeeb, Animate Dead, Magic Missile, Color Spray, Obscuring Mist, Cause Fear, Sleep
			# Wooden Elvish Chain 6073
			# Shield + 1
			# Silver Banded Mail 6120
			# Lareth's Breastplate 6097
			# Lareth's Staff 4120
			# Lareth's Plate Boots 6098
			# Lareth's Ring 6099
			# Fungus Figurine 12024
			# Shortsword +1 4126
			# MW Xbow 4177
			# Cloak of Elvenkind x2 6058
			# ~10k GP before spending
			# Shopping:
			# MW Scimitar 400GP - 4048
			# MW Greataxe 400GP  - 4065
			# MW Greatsword 400GP  - 4012
			# Prot from arrows 180GP - 9367
			# Resist Energy 180GP - 9400
			# Web 180 GP- 9531
			# Summon Mon 1 30GP - 9467

			#if pc.money_get() < 10000 * 100:
			#	pc.money_adj(10000 * 100 - pc.money_get())
			if which_class(pc) == 'fighter':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4010) # Greatsword

			if which_class(pc) == 'barbarian':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4064) # Greataxe
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6055) # Barbarian Armor

			if which_class(pc) == 'cleric':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4070) # Morningstar
				if giv(pc, 6073, 1) == 0:
					giv(pc, 6070) # Large Wooden Shield
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6153) # Fine Scalemail

			if which_class(pc) == 'druid':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4045) # Scimitar
				if giv(pc, 6073, 1) == 0:
					giv(pc, 6070) # Large Wooden Shield
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6009) # Druid Hide


			if which_class(pc) == 'monk':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff

			if which_class(pc) == 'bard':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6013) # Brown Leather Armor
				giv(pc, 12564) # Mandolin
				#giv(pc, 6031, 1) # Eyeglasses
				giv(pc, 12675, 1) # Merchant's Scale

			if which_class(pc) == 'sorcerer':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff

			if which_class(pc) == 'rogue':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				#giv(pc, 6031, 1) # Eyeglasses
				giv(pc, 12675, 1) # Merchant's Scale
				if pc.item_worn_at(5) == OBJ_HANDLE_NULL:
					giv(pc, 6042) # Black Leather Armor
				giv(pc, 12012) # Thieves Tools
				giv(pc, 12767) # Lockslip Grease

			if which_class(pc) == 'wizard':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff
				giv(pc, 9229) # Grease Scroll
				giv(pc, 12848) # Scholar's kit
				#giv(pc, 6031, 1) # Eyeglasses
				giv(pc, 12675, 1) # Merchant's Scale

		pc.identify_all()	
		pc.item_wield_best_all()

		
	#giv(pc, 6031, 1) # Eyeglasses
	giv(pc, 12675, 1) # Merchant's Scale
	giv(pc, 12012, 1) # Thieves Tools
	giv(pc, 12767, 1) # Lockslip Grease


	return	
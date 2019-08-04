from toee import *
from __main__ import game
from utilities import *
from Co8 import *
#from itt import *
from array import *
from math import atan2
from py00439script_daemon import *
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
	gearup(simulated_game_state, cheat_items = 1)
	print str(simulated_game_state)

	if simulated_game_state >= 0:
		game.quests[18].state = qs_completed # Catch Furnok quest
		game.quests[100].state = qs_accepted # Fetch Giant's head
		game.global_flags[21] = 1 # Enable Terjon
		game.areas[2] = 1 # Moathouse
		game.areas[5] = 1 # Emridy

	if simulated_game_state >= 1:
		print 'Executing WB...'
		# Having just completed Welkwood Bog, going on Moathouse + Emridy


		game.story_state = 1

		game.areas[7] = 1 # Welkwood Bog


		game.global_vars[970] = 2 # Talked to Smyth about WB

		# game.global_flags[66] = 1 # Paid Elmo - do NOT set this flag, else he won't get his better gear
		game.global_flags[67] = 1 # Have spoken to vignette's relevant figure
		game.global_flags[605] = 1 # WB description box fired 
		game.global_flags[976] = 1 # Mathel dead

		game.quests[73].state = qs_completed # Welkwood Bog quest
		if game.party_alignment == TRUE_NEUTRAL:
			game.quests[27].state = qs_accepted # Find Terjon's pendant
		
		################
		# Kill section #
		################
		if get_v('qs_welkwood') & ((2**11) - 1) != ((2**11) - 1):
			set_v('qs_welkwood', get_v('qs_welkwood') | 2**0)
			if get_v('qs_welkwood') & 2**1 == 0:
				cnk(14785) # Mathel
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**1)

			if get_v('qs_welkwood') & 2**2 == 0:
				cnk(14183) # Goblin Leader
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**2)

			if get_v('qs_welkwood') & 2**3 == 0:
				cnk(14641) # Kobold Sergeant
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**3)

			if get_v('qs_welkwood') & 2**4 == 0:
				cnk(14631) # Gnoll
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**4)

			if get_v('qs_welkwood') & 2**5 == 0:
				cnk(14081) # Skeleton Gnoll
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**5)

			if get_v('qs_welkwood') & 2**6 == 0:
				cnk(14640, how_many = 10, timer = 200) # Kobolds	
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**6)

			if get_v('qs_welkwood') & 2**7 == 0:
				cnk(14187, how_many = 18, timer = 800) # Goblins
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**7)

			if get_v('qs_welkwood') & 2**8 == 0:
				cnk(14183) # Goblin Leader
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**8)

			if get_v('qs_welkwood') & 2**9 == 0:
				cnk(14640, how_many = 9, timer = 1800) # Kobolds
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**9)

			if get_v('qs_welkwood') & 2**10 == 0:
				cnk(14641) # Kobold Sergeant
				set_v('qs_welkwood', get_v('qs_welkwood') | 2**10)

			print 'WB executed!'

		#for pc in game.party[0].group_list():
		#	if pc.stat_level_get(stat_experience) <= 820:
		#		pc.stat_base_set(stat_experience, 820)


	if simulated_game_state >= 2:
		if autokill_on == 1:
			set_f('qs_autokill_moathouse', 1)
		# Having just completed Moathouse + Emridy + Welkwood Bog
		#for pc in game.party[0].group_list():
		#	if pc.stat_level_get(stat_experience) <= 6000:
		#		pc.stat_base_set(stat_experience, 6000)
		game.story_state = 2
		game.areas[8] = 1 # Moathouse Cave Exit
		game.areas[10] = 1 # Deklo
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
		game.areas[9] = 1 # HB
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
		game.areas[4] = 1 # Temple
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
	
	for pc in game.party:
		if which_class(pc) == 'fighter':
			figh_pc = pc
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
				giv(pc, 12677, 1) # Spyglass

			if which_class(pc) == 'sorcerer':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff
				if cheat_items == 1:
					giv(pc, 12620) # Wand of Fireball (10th)
					giv(pc, 12262) # Wand of Knock

			if which_class(pc) == 'rogue':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				giv(pc, 6031) # Eyeglasses
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
				giv(pc, 6031, 1) # Eyeglasses
				giv(pc, 12675, 1) # Merchant's Scale

			if which_class(pc) == 'sorcerer':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4243) # Quarterstaff

			if which_class(pc) == 'rogue':
				if pc.item_worn_at(3) == OBJ_HANDLE_NULL:
					giv(pc, 4060) # Dagger
				giv(pc, 6031, 1) # Eyeglasses
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
				giv(pc, 6031, 1) # Eyeglasses
				giv(pc, 12675, 1) # Merchant's Scale

		pc.identify_all()	
		pc.item_wield_best_all()

		
	giv(pc, 6031, 1) # Eyeglasses
	giv(pc, 12675, 1) # Merchant's Scale
	giv(pc, 12012, 1) # Thieves Tools
	giv(pc, 12767, 1) # Lockslip Grease


	return	
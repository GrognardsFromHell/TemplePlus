from toee import *
from __main__ import game
from marc import *

def chapter_one():
	game.story_state = 1
	game.quests[1].state = qs_accepted
	starting_money()
	chapter_one_slides()

def chapter_one_slides():
	show_chapter_slides([401,402,403,404,411,412])

def chapter_two_ready():
	if game.story_state == 1:
		# 8x GM quests, 6x captain quests, 2x church quests, big love
		for quest in (5,6,7,8,9,10,11,12,64,65,66,67,68,69,71,72,78):
			if game.quests[quest].state == qs_accepted:
				return 0  # these quests must be completed if accepted
		if (
			game.quests[2].state >= qs_completed and  # lost in his own museum
			game.quests[3].state >= qs_completed and  # at a crossroads
			game.quests[4].state >= qs_completed      # the unknown
		):
			return 1
	return 0

def chapter_two():
	game.story_state = 2
	game.quests[1].state = qs_completed
	game.quests[21].state = qs_accepted
	reset_inventory_restock_flags()
	reset_random_encounters()
	if game.global_vars[74] in (1,2,3,4):
		game.encounter_queue.append (10 + game.global_vars[74])
	guildmaster_exit(2)  # sets ggv41 and ggv20
	chapter_two_slides()
	return_to_inn(2)

def chapter_two_slides():
	slides = [501]
	if game.global_vars[74] in (1,2,3,4):  # trenn, rhondi, war, or peace
		slides += [510 + game.global_vars[74]]
	if game.global_vars[41] in (1,2,3,5,6,7):  # vorlo, bex, gil, myella, peck, magnolia
		slides += [520 + game.global_vars[41]]
	slides += [531]
	show_chapter_slides(slides)

def chapter_three_ready():
	if game.story_state == 2:
		if (
			# game.quests[61].state >= qs_completed and  # glyphs for riss
			(game.quests[22].state >= qs_completed or game.global_vars[41] == 1) and  # vorlo
			(game.quests[24].state >= qs_completed or game.global_vars[41] == 2) and  # bex
			(game.quests[26].state >= qs_completed or game.global_vars[41] == 3) and  # gil
			(game.quests[28].state >= qs_completed or gbget(623,21) or gbget(623,25)) and  # vendetta
			(game.quests[30].state >= qs_completed or game.global_vars[41] == 5) and  # myella
			(game.quests[32].state >= qs_completed or game.global_vars[41] == 6) and  # peck
			(game.quests[34].state >= qs_completed or game.global_vars[41] == 7)  # magnolia
		):
			return 1
	return 0

def chapter_three():
	game.story_state = 3
	game.quests[21].state = qs_completed
	game.quests[41].state = qs_accepted
	reset_inventory_restock_flags()
	reset_random_encounters()
	guildmaster_exit(3)  # sets ggv42 and ggv20
	chapter_three_slides()
	return_to_inn(3)
	
def chapter_three_slides():

	slides = [601]

	# GM Murdered
	if game.global_vars[42] in (1,2,3,5,6,7):  # vorlo, bex, gil, myella, peck, magnolia
		slides += [620 + game.global_vars[42]]

	# No normal GM's remains
	if guildmaster_count() == 0:
		if gbget(20,4) == 0 and gbget(623,1) == 0:  # vendetta ok
			slides += [634]
		else:
			slides += [630]

	# One normal GM remains
	elif guildmaster_count() == 1:
		if gbget(20,4) == 0 and gbget(623,1) == 0: # vendetta ok
			slides += [638]
		else:
			for gm in (1,2,3,5,6,7):
				if game.global_vars[20] & 2**gm == 0:
					slides += [630 + gm]
					break

	if game.global_vars[74] in (1,2,3,4):  # trenn, rhondi, war, or peace
		slides += [610 + game.global_vars[74]]

	slides += [641]
	show_chapter_slides(slides)

def chapter_four_ready():
	if game.story_state == 3:
		if (
			game.quests[42].state >= qs_completed and  # deeper
			game.global_vars[45] >= 1                  # leader chosen
		):
			return 1
	return 0

def chapter_four():
	game.story_state = 4
	game.quests[41].state = qs_completed
	game.quests[46].state = qs_accepted
	reset_inventory_restock_flags()
	reset_random_encounters()
	chapter_four_slides()
	return_to_inn(4)
	
def chapter_four_slides():
	slides = [701,702,703]
	show_chapter_slides(slides)

def show_chapter_slides (slides_list):
	for s in slides_list:
		game.moviequeue_add(s)
	game.moviequeue_play()

def return_to_inn(chapter):	
	rest_time = 365 * 24 * 60 * 60
	if game.is_daytime():
		rest_time += 12 * 60 * 60
	game.fade_and_teleport(rest_time,0,0,5006,494,489)
	game.sound(4317,1)
	game.sound(4318,chapter)
	for pc in game.party:
		if not pc.is_category_type(mc_type_undead):
			pc.heal( OBJ_HANDLE_NULL, dice_new(str(20) + "d" + str(20)) )

#------------------------------------------------------------------------------
# Counts the number of Guildmasters still in the guild, EXCLUDING VENDETTA.
# Based on value of global variable 20.
#------------------------------------------------------------------------------
def guildmaster_count():
	gm_count = 0
	for gm in (1,2,3,5,6,7):
		if gbget(20,gm) == 0:
			gm_count += 1
	return gm_count
	
# Guildmaster Exit
#
# Chapter One:
#   One random guildmaster will depart at the start of chapter 2.
#   They are chosen from a list of guildmasters the PC didn't do a quest for.
#   The GM must also have Not run off, and must also Not be dead.
#   At this point, no GM should have run off or be dead yet, but I'm checking anyways.
#   If PC did a quest for each GM, Bex disappears as the default.
#
# {5} {This Old House}
# {6} {She's Good With Wood}
# {7} {All the Beautiful Colors}
# {8} {Sword of Evil}
# {9} {Goblin Purge}
# {10} {Kaleela the Teenage Witch}
# {11} {Armed Escort}
# {12} {Here Kitty Kitty}
#
# Chapter Two:
#   One random guildmaster will die at the start of chapter 3.
#   If only one is left, or none are left, there is no murder.
#   They are chosen from a list of guildmasters still in the Guild.
#
# {22} {Mr. Temm's Excellent Adventure}
# {24} {Toll Trolls}
# {26} {Luxwood}
# {28} {Thin the Herd}
# {29} {Paladin's Fall}
# {30} {Three Birds}
# {32} {Pirates of the Coast}
# {34} {A Thief and the Knight}
#
# game.global_vars[41] marks the one guildmaster that departs when Chapter 2 begins. Not dead.
# game.global_vars[42] marks the one guildmaster murdered when Chapter 3 begins. Dead.
# game.global_vars[20] marks all guildmasters no longer in the guildhall for any reason.
#

def guildmaster_exit(chapter):

	guildmasters = []
	gm_list = [1,2,3,5,6,7]
	quest_dict = {1:5, 2:6, 3:7, 5:10, 6:11, 7:12}
	proto_dict = {1:620, 2:621, 3:622, 5:624, 6:625, 7:626}

	if chapter == 2:
	
		# Make list of all GM's that PC didn't complete a quest for.
		for gm in gm_list:
			quest = quest_dict[gm]
			proto = proto_dict[gm]
			if game.quests[quest].state != qs_completed:
				if gbget(20,gm) == 0 and gbget(proto,1) == 0:
					guildmasters += [gm]
					
		# GM resigned
		if len(guildmasters) >= 1:
			gm_exit = game.random_range (0, len(guildmasters)-1)
			game.global_vars[41] = guildmasters[gm_exit]
		else:
			game.global_vars[41] = 2  # default to bex

		# Update the global that holds the list of all GM's that are no longer in the Guild 
		game.global_vars[20] = game.global_vars[20] | (2**game.global_vars[41])

	if chapter == 3:

		# Make list of all GM's that are still in the Guild. Not including Vendetta.
		for gm in gm_list:
			if game.global_vars[20] & 2**gm == 0:
				guildmasters += [gm]
					
		# GM murdered
		if len(guildmasters) >= 2:
			gm_exit = game.random_range (0, len(guildmasters)-1)
			game.global_vars[42] = guildmasters[gm_exit]
			game.global_vars[20] = game.global_vars[20] | (2**game.global_vars[42])
			proto = proto_dict[game.global_vars[42]]
			gbset(proto,1)  # mark gm as dead
		else:
			game.global_vars[42] = 0  # no GM dies


# Merchant restock
#
# These merchants only restock at the start of a new chapter,
# not at dawn like the other merchants.
# Mostly, they are magic merchants, or sell rare items like Mithral.
#
# landy(502)		mw items
# brollo(503)		magic
# hannah(515)		magic
# shikora(529)		oriental weapons
# drasindra(540)	magic
# daerwyg(541)		mithral
# corona(560)		magic

def reset_inventory_restock_flags():
	for inv in [502,503,515,529,540,541,560]:
		if game.global_vars[inv] & 4: 
			game.global_vars[inv] = game.global_vars[inv] - 4
	for inv in [100]:  # poor boxes
		game.global_flags[inv] = 0

def reset_random_encounters():
	for gv in range(700,714):
		game.global_vars[gv] = 0

def starting_money():
	game.party[0].money_adj(-game.party[0].money_get())
	for pc in game.party:
		if (pc.stat_level_get(stat_level_barbarian)):
			pc.money_adj(10000)   # dice = dice_new ('4d4')
		elif (pc.stat_level_get(stat_level_bard)):
			pc.money_adj(10000)   # dice = dice_new ('4d4')
		elif (pc.stat_level_get(stat_level_cleric)):
			pc.money_adj(12500)   # dice = dice_new ('5d4')
		elif (pc.stat_level_get(stat_level_druid)):
			pc.money_adj(5000)   # dice = dice_new ('2d4')
		elif (pc.stat_level_get(stat_level_fighter)):
			pc.money_adj(15000)   # dice = dice_new ('6d4')
		elif (pc.stat_level_get(stat_level_monk)):
			pc.money_adj(1250)   # dice = dice_new ('5d4')
		elif (pc.stat_level_get(stat_level_paladin)):
			pc.money_adj(15000)   # dice = dice_new ('6d4')
		elif (pc.stat_level_get(stat_level_ranger)):
			pc.money_adj(15000)   # dice = dice_new ('6d4')
		elif (pc.stat_level_get(stat_level_rogue)):
			pc.money_adj(12500)   # dice = dice_new ('5d4')
		elif (pc.stat_level_get(stat_level_sorcerer)):
			pc.money_adj(7500)   # dice = dice_new ('3d4')
		elif (pc.stat_level_get(stat_level_wizard)):
			pc.money_adj(7500)   # dice = dice_new ('3d4')
		elif (pc.stat_level_get(stat_level_favored_soul)):  #Added by temple+
			pc.money_adj(12500)   # dice = dice_new ('5d4')  #Added by temple+
		elif (pc.stat_level_get(stat_level_scout)):  #Added by temple+
			pc.money_adj(12500)   # dice = dice_new ('5d4')  #Added by temple+
		elif (pc.stat_level_get(stat_level_warmage)):  #Added by temple+
			pc.money_adj(10000)   # dice = dice_new ('4d4')  #Added by temple+
		elif (pc.stat_level_get(stat_level_beguiler)):  #Added by temple+
			pc.money_adj(15000)   # dice = dice_new ('6d4')  #Added by temple+
		elif (pc.stat_level_get(stat_level_swashbuckler)):  #Added by temple+
			pc.money_adj(15000)   # dice = dice_new ('6d4')  #Added by temple+
	return

# Test Functions, for chapter transition 
# Completes the quests needed to start a new chapter. 
# These do Not actually start the new chapter.

def ct2(all=0):
	r = 4
	if all:
		r = 3
	for c in (2,3,4):
		game.quests[c].state = qs_completed
	game.global_vars[74] = game.random_range(1,4)
	for q in (5,6,7,10,11,12):
		roll = game.random_range(1,r)
		float_num(game.leader,roll)
		if roll in (1,2,3):
			game.quests[q].state = qs_accepted
			if game.random_range(1,r) in (1,2,3):
				game.quests[q].state = qs_completed
			else:
				game.quests[q].state = qs_botched

def ct3():
	game.quests[61].state = qs_completed
	for gm in ((1,22),(2,24),(3,26),(4,28),(5,30),(6,32),(7,34)):
		if game.global_vars[41] != gm[0]:  # exclude gm that left in chapter 1
			game.quests[gm[1]].state = qs_accepted
			if game.random_range(1,4) in (1,2,3):
				game.quests[gm[1]].state = qs_completed
			else:
				game.quests[gm[1]].state = qs_botched

def ct4():
	game.quests[42].state = qs_completed  # deeper
	game.quests[43].state = qs_completed  # guildhall reformation




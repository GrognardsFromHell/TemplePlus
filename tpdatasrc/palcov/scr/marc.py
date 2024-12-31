from toee import *
from math import *
from time import *
from utilities import *
from scripts import *
from inventory import *

#------------------------------------------------------------------------------
# Creates an object in the game at a specified location, with or w/o inventory.
#
# proto - The proto ID for the object to be created.
# loc - The location to create the object, via location_from_axis(x,y).
# inv - The source of the inventory.
#    0: use the inventory source number defined by that object's proto. The
#       inventory is listed in the dictionary inv_source{}, via inv_source.txt.
#    n: use this inventory source number instead.
#   -1: don't add any inventory.
# kos - Set the ONF_KOS flag in this manner.
#    0: don't change it   1: set ONF_KOS   2: set ONF_KOS_OVERRIDE 
#------------------------------------------------------------------------------
def create_obj (proto, loc, inv=0, kos=0):

	obj = game.obj_create (proto, loc)

	if obj:
		if inv >= 0:
			create_inv (obj, inv)
			npcbit_1(obj,0)  # blocks creating of inventory in san_first_heartbeat
		if obj.type == obj_t_npc:
			if kos == 1:
				obj.npc_flag_set(ONF_KOS)
				obj.npc_flag_unset(ONF_KOS_OVERRIDE)
			elif kos == 2:
				obj.npc_flag_set(ONF_KOS_OVERRIDE)

	return obj

#------------------------------------------------------------------------------
# Creates the inventory for an NPC or container, found in inv_source.txt.
#
# obj - The obj handle of the critter or container.
# inv - The source of the inventory.
#    0: use the inventory source number defined by that object's proto. The
#       inventory is listed in the dictionary inv_source{}, via inv_source.txt.
#    n: use this inventory source number instead.
# clear - Clear out the objects existing inventory?
#    0: leave the existing inventory.
#    1: clear the existing inventory.
#------------------------------------------------------------------------------
def create_inv (obj, inv=0, clear=0):

	if clear == 1:
		del_inv(obj)

	adjust = 0

	if inv == 0:
		if obj.type == obj_t_npc:
			inv = obj.obj_get_int(obj_f_critter_inventory_source)
			adjust = 1
		elif obj.type == obj_t_container:
			inv = obj.obj_get_int(obj_f_container_inventory_source)

	if inv in inv_source.keys():
		for x in inv_source[inv]:
			roll = game.random_range(1,100)
			for y in x:
				if roll <= y[0]:
					if len(y) == 2:
						add_to_inv(obj, y[1], 0, 0, 0, adjust)
					elif len(y) == 4:
						add_to_inv(obj, y[1] ,y[2], y[3], 0, adjust)
					elif len(y) == 5:
						add_to_inv(obj, y[1] ,y[2], y[3], y[4], adjust)
					break

	if obj.type == obj_t_npc:
		obj.item_wield_best_all()

	return

#------------------------------------------------------------------------------
# Adds items to the inventory of an NPC, PC, or Container. The function may
# call itself recursively as it processes treasure tables.
#
# obj - The obj handle of the critter or container.
#
# items - The items that will be added to the inventory. There are five possible
#         ways to format the request, making the function multi-purpose.
#
#    1. Proto number, int - an item in protos.tab.
#         example: (obj, 6104)
#
#    2. Proto description, str - a proto referenced by its description, found
#       in the dictionary called descriptions{} in descriptions.py.
#         example: (obj, 'dagger +2')
#
#    3. Treasure table, str - one item will be rolled from the given treasure
#       table found in inv_tables.txt.
#         example: (obj, 'POTION MINOR')
#
#    4. Combo - a list or tuple of any number of items, in any of the 3 formats
#       described above. All items will be added.
#         example: (obj, [6104, 'dagger +2', 'combat boots', 'POTION MINOR'])
#         example: (obj, ['composite longbow str 18', 'quiver of arrows'])
#         example: (obj, [8013, 8101, 8113, 'FULL PLATE 0'])
#
#    5. Special names, str - The following names trigger a special process, and
#       do not reference a specific proto or treasure table. The die roll
#       parameters may also be used to mean something different for each one:
#
#        'CGI' - adds Coins, Goods & Items based EL, see function add_cgi().
#           die_qty is the EL, die_size is fraction, die_add is the multiplier.
#        'COINS BY VALUE' - adds coins, die roll is total value in CP.
#        'GEMS BY VALUE' - adds gems, die roll is total value in GP.
#        'JEWELRY BY VALUE' - adds jewelry, die roll is total value in GP.
#
# die_qty - number of dice to roll
# die_size - size of the die
# die_add - number to add to the roll
#
#    These arguments are optional, and define a dice roll that will give the
#    total number of each item added to the inventory.
# 
#    Ex: (obj, 12010, 2,4,1) adds 2d4+1 Emeralds. (not 2 to 4 + 1)
#    Ex: (obj, 'emerald', 0,0,3) adds 3 Emeralds.
#    Ex: (obj, [12010,'GEMS'], 1,4,0) adds 1d4 Emerald, 1d4 items from 'GEMS'.
#    Ex: (obj, [12010,12023,'dagger']) adds 1 Emerald, 1 Ruby Brooch, 1 dagger.
#
# adjust - Adjust a treasure table before rolling, if it ends in 0,1,2, or 3.
#
#    This is optional. See the comments in adjust_table() for details.
#    This is only used for a treasure table, or a combo with a treasure table.
#    Any treasure tables rolled under 'CGI' are not adjusted.
#
# unique - If this is set to -1, create the item only if it is not already in
#          the container's inventory.
#
# loc - If not zero, create the item on the ground.
#
# VERY IMPORTANT: Adding items to a container with create_item_in_inventory(),
# which calls npc.item_get(item), can only add 120 total items before it won't
# work anymore (any new item will be left on the ground). However, when
# inserted manually in the game, it can hold 200 total items before filling up.
#------------------------------------------------------------------------------
def add_to_inv (obj, items, die_qty=0, die_size=0, die_add=0, adjust=1, unique=0, loc=0):

	# dbug("items",items)
	
	qty = 1
	if die_size or die_add:
		qty = max (roll_dice(die_qty, die_size, die_add), 0)

	handle = OBJ_HANDLE_NULL

	if items in ['CGI','COINS BY VALUE','GEMS BY VALUE','JEWELRY BY VALUE']:
		if items == 'CGI':
			for c in range(0, die_add):
				add_cgi(obj, die_qty, die_size) # obj, EL, frac
		elif items == 'COINS BY VALUE':
			add_coins(obj, qty)
		elif items == 'GEMS BY VALUE':
			gj = CalcGJ('gems', qty)
			add_to_inv(obj, gj)
		elif items == 'JEWELRY BY VALUE':
			gj = CalcGJ('jewelry', qty)
			add_to_inv(obj, gj)
		return

	for i in range(0, qty):

		# proto by number
		if isinstance(items,int):
			if items > 4000 and items < 13000:
				if loc == 0:
					if unique == 0 or not obj.has_item(items):
						create_item_in_inventory(items, obj)
				else:
					handle = game.obj_create(items,loc)

		# proto by description
		elif isinstance(items,str) and items.islower():
			# TBD, check if the damn name exists in descriptions!! 
			if loc == 0:
				if unique == 0 or not obj.has_item(descriptions[items]):
					create_item_in_inventory (descriptions[items], obj)
			else:
				handle = game.obj_create(descriptions[items],loc)

		# treasure table, roll the proto
		elif isinstance(items,str):
			if adjust == 1 and items[-2:] in [' 0',' 1',' 2',' 3']:
				items = items[0:-2] + adjust_table(items[-1])
				adjust = 0  # don't adjust sub-tables
			roll = game.random_range(1,100)
			for i in inv_tables[items]:
				if roll <= i[0]:
					handle = add_to_inv(obj,i[1],0,0,0,adjust,unique,loc)
					break

		# multiple items in a tuple or list
		elif isinstance(items,tuple) or isinstance(items,list):
			if loc == 0:
				for i in items:
					add_to_inv(obj,i,0,0,0,adjust,unique,loc)
			else:
				handle = add_to_inv(obj,items[0],0,0,0,adjust,unique,loc)
			
	# Return object handle if it is a single item spawned on the ground
	if handle != OBJ_HANDLE_NULL:
	# if isinstance(items,int) or (isinstance(items,str) and items.islower()) or isinstance(items,str):
		if qty == 1 and loc != 0:
			return handle

#------------------------------------------------------------------------------
# Adds one item to the ground, at the given location.
# If the item is a tuple, it will only add the first item to the ground.
# A tuple item is usually a bow with arrows, so the arrows won't be made.
#------------------------------------------------------------------------------
def add_to_ground (items, loc, adjust=0):
	handle = add_to_inv (OBJ_HANDLE_NULL, items, 0, 0, 1, adjust, 0, loc) 
	return handle

#------------------------------------------------------------------------------
# Adjusts a treasure table ending with ' 0', ' 1', ' 2', or ' 3', so that the
# table actually used can become better or worse. The purpose here is to create
# a realistic variation for similar creatures. So a group of 3rd level fighters
# equipped with 'LONGSWORD 0' will usually get 'masterwork longsword' but may
# occasionally get 'longsword +1' or a regular 'longsword', and a small chance
# to get a +2 or +3. This is currently only being used for weapons and armor
# directly specified as an NPCs main equipment, and not for general random
# treasure generated by 'CGI'. A treasure table can not adjust up past 3.
#
# To clarify, a treasure table ending in ' 0' will roll a masterwork item,
# while those ending in ' 1', ' 2',or ' 3' will roll a +1, +2, or +3 weapon
# respectively.
#
# The chances for adjustment are currently set at:
#   11% adjust up by 1,   3% adjust up by 2,   1% adjust up by 3 
#   11% adjust down by 1,   3% adjust up down 2,   1% adjust down by 3 
# 
# Example: 'LONGSWORD 0' has a chance of adjusting up or down:
#   11% chance to adjust up by 1, and become 'LONGSWORD 1'.
#    3% chance to adjust up by 2, and become 'LONGSWORD 2'.
#    1% chance to adjust up by 3, and become 'LONGSWORD 3'.
#   11% chance to adjust down by 1, and become 'LONGSWORD'.
#    3% chance to adjust down by 2, and become 'LONGSWORD'.
#    1% chance to adjust down by 3, and become 'LONGSWORD'.
#   70% chance it will remain a Masterwork Longsword.
#------------------------------------------------------------------------------
def adjust_table (table_num_str):

	n = int(table_num_str)
	adjustments = ( (1,-3), (4,-2), (15,-1), (26,1), (29,2), (30,3) )

	roll = game.random_range(1,100)
	for a in adjustments:
		if roll <= a[0]:
			n = n + a[1] 
			break

	if n >= 0:
		return ' ' + str(min(3,n))
	return ''

#------------------------------------------------------------------------------
# Adds random treasure to the inventory of an NPC, PC, or container based on
# the given encounter level, and in the form of coins, goods, and items.
# NONE OF THESE TABLES WILL BE ADJUSTED by adjust_table().
#
# obj - The obj handle of the critter or container.
# el - The encounter level, as described in DMG pages 52-53.
# fraction - Reduces the treasure to 1/fraction. (1/2, 1/5, etc.)
# 
# If the object is an NPC, it adds the name 'PERSONAL' to the end of the table
# name, which uses treasure tables that differ in the following ways:
#
#   1. Any rolls of weapons and armor are ignored, as those items should be
#      specified directly in the inven_src of every NPC.
#   2. Potions will only be rolled up that can be useful in combat. Since most
#      strategies have a 'use potion' command, a potion such as Remove Fear or
#      Resist Acid will almost always be wasted when used. 
#  
# Format of dictionary cgi:
#   EL : ( (Coins %, die_qty, die_size, die_plus),		# [0]
#	   (Gems %, die_qty, die_size, die_plus),			# [1]
#	   (Jewelry %, die_qty, die_size, die_plus),		# [2]
#	   (Mundane %, die_qty, die_size, die_plus),		# [3]
#	   (Magic Minor %, die_qty, die_size, die_plus),	# [4]
#	   (Magic Medium %, die_qty, die_size, die_plus),	# [5]
#	   (Magic Major %, die_qty, die_size, die_plus)	)	# [6]
#------------------------------------------------------------------------------
cgi = { 
1:  ( (15,1,150,0),  (91,0,0,1),  (96,0,0,1),  (72,0,0,1),  (96,0,0,1),  (101,0,0,0), (101,0,0,0) ),
2:  ( (14,1,300,0),  (82,1,3,0),  (96,1,3,0),  (50,0,0,1),  (86,0,0,1),  (101,0,0,0), (101,0,0,0) ),
3:  ( (12,1,450,0),  (78,1,3,0),  (96,1,3,0),  (50,1,3,0),  (80,0,0,1),  (101,0,0,0), (101,0,0,0) ),
4:  ( (12,1,600,0),  (71,1,4,0),  (96,1,3,0),  (43,1,4,0),  (63,0,0,1),  (101,0,0,0), (101,0,0,0) ),
5:  ( (11,1,750,0),  (61,1,4,0),  (96,1,4,0),  (58,1,4,0),  (68,1,3,0),  (101,0,0,0), (101,0,0,0) ),
6:  ( (11,1,900,0),  (57,1,4,0),  (93,1,4,0),  (55,1,4,0),  (60,1,3,0),  (100,0,0,1), (101,0,0,0) ),
7:  ( (12,1,1200,0), (49,1,4,0),  (89,1,4,0),  (101,0,0,0), (52,1,3,0),  (98,0,0,1),  (101,0,0,0) ),
8:  ( (11,1,1500,0), (46,1,6,0),  (86,1,4,0),  (101,0,0,0), (49,1,4,0),  (97,0,0,1),  (101,0,0,0) ),
9:  ( (11,1,1800,0), (41,1,8,0),  (81,1,4,0),  (101,0,0,0), (44,1,4,0),  (92,0,0,1),  (101,0,0,0) ),
10: ( (11,1,2700,0), (36,1,8,0),  (80,1,6,0),  (101,0,0,0), (41,1,4,0),  (89,0,0,1),  (100,0,0,1) ),
11: ( (9,1,3600,0),  (25,1,10,0), (75,1,6,0),  (101,0,0,0), (32,1,4,0),  (85,0,0,1),  (99,0,0,1) ),
12: ( (9,1,4800,0),  (18,1,10,0), (71,1,8,0),  (101,0,0,0), (28,1,4,0),  (89,0,0,1),  (98,0,0,1) ),
13: ( (9,1,6000,0),  (12,1,12,0), (67,1,10,0), (101,0,0,0), (20,1,6,0),  (74,0,0,1),  (96,0,0,1) ),
14: ( (9,1,8000,0),  (12,2,8,0),  (67,2,6,0),  (101,0,0,0), (20,1,6,0),  (59,0,0,1),  (93,0,0,1) ),
15: ( (4,1,12000,0), (10,2,10,0), (66,2,8,0),  (101,0,0,0), (12,1,10,0), (47,0,0,1),  (91,0,0,1) ),
16: ( (4,1,14000,0), (8,4,6,0),   (65,2,10,0), (101,0,0,0), (41,1,10,0), (47,1,3,0),  (91,0,0,1) ),
17: ( (4,1,17000,0), (5,4,8,0),   (64,3,8,0),  (101,0,0,0), (101,0,0,0), (34,1,3,0),  (84,0,0,1) ),
18: ( (3,1,22000,0), (5,3,12,0),  (55,3,10,0), (101,0,0,0), (101,0,0,0), (25,1,4,0),  (81,0,0,1) ),
19: ( (3,1,25000,0), (4,6,6,0),   (51,6,6,0),  (101,0,0,0), (101,0,0,0), (5,1,4,0),   (71,0,0,1) ),
20: ( (3,1,30000,0), (3,4,10,0),  (39,7,6,0),  (101,0,0,0), (101,0,0,0), (26,1,4,0),  (66,1,3,0) )
} 

def add_cgi (container, el, fraction):

	cgi_tables = { 1:'GEMS', 2:'JEWELRY', 3:'MUNDANE', 4:'MAGIC MINOR', 5:'MAGIC MEDIUM', 6:'MAGIC MAJOR' }

	if el not in range(1,21) or fraction not in range (1,51):
		return

	# Coins (0)
	coins = cgi[el][0]
	roll = game.random_range(1,100)
	if roll >= coins[0]:
		qty = roll_dice (coins[1], coins[2], coins[3]) * 100
		qty = qty / fraction
		add_coins (container, qty)

	# Goods (jewelry(2), gems(1))
	# Roll twice to allow for the possibility of both gems AND jewelry.
	# Double the fraction to compensate for rolling twice. 
	for n in range(0,2):
		roll = game.random_range(1,100)
		for i in [2,1]:
			good = cgi[el][i]
			if roll >= good[0]:
				table = cgi_tables[i] 
				qty = roll_dice (good[1], good[2], good[3])
				add_to_inv_fractioned (container, table, qty, fraction*2)
				break

	# Items (major(6), medium(5), minor(4), mundane(3))
	roll = game.random_range(1,100)
	for i in [6,5,4,3]:
		item = cgi[el][i]
		if roll >= item[0]:
			table = cgi_tables[i]
			if container.type in [obj_t_npc, obj_t_pc]:
				table += ' PERSONAL'
			qty = roll_dice (item[1], item[2], item[3])
			add_to_inv_fractioned (container, table, qty, fraction)
			break

#------------------------------------------------------------------------------
# Reduces the qty of the items added to the inventory by the given fraction.
# It doesn't divide the qty directly, like it does for coins, but instead
# uses the fraction as a percent chance that each item will be included.
# It is done this way since it would be complicated to spilt certain treasures.
#
# Ex: A treasure of 'potion of haste' and '+1 dagger' can not be fractioned
# by 6, so instead each one is given a 1/6 chance of being included.  
#------------------------------------------------------------------------------
def add_to_inv_fractioned (container, items, qty, fraction):

	dbug ("items",items)
	dbug ("   qty",qty)
	dbug ("   fraction",fraction)

	for q in range(0, qty):
		if game.random_range(1, fraction) == 1:
			dbug ("   YES")
			add_to_inv (container, items, 0, 0, 0, 0)  # adjust = 0
		else:
			dbug ("   NO")

#------------------------------------------------------------------------------
# Adds a value of coins to a container and split it into cp, gp, sp and pp, so
# that it looks diverse. The total value of the coins will not change.  
#
# container - The obj handle of the critter or container.
# coins_cp - The total value of the coins, in copper pieces.
#------------------------------------------------------------------------------
def add_coins (container, coins_cp, max_types=0):

	low_pp = (0,0,0,10,25,50,75,90,95,99)
	low_gp = (0,0,0,10,25,50,75,90,95,99)
	low_sp = (0,0,0,10,25,50,75,90,95,99)
	cp,sp,gp,pp = coins_cp,0,0,0
	types = 0
	
	# One in 10 chance to allow more than 2 types of coins
	if max_types not in (1,2,3,4):
		max_types = 2
		if game.random_range(1,10) == 1:
			# print "4 types allowed"
			max_types = 4

	low = low_pp[game.random_range(0,9)]
	if cp >= 1000 and low > 0:
		types += 1
		if types < max_types:
			percent = game.random_range(low,100)
			pp = int( (cp/1000) * percent/100.0 )
			cp = cp - pp*1000  
		else:
			pp = cp/1000
			cp = 0
			
	low = low_gp[game.random_range(0,9)]
	if cp >= 100 and low > 0:
		types += 1
		if types < max_types:
			percent = game.random_range(low,100)
			gp = int( (cp/100) * percent/100.0 )
			cp = cp - gp*100  
		else:
			gp = cp/100
			cp = 0
			
	low = low_sp[game.random_range(0,9)]
	if cp >= 10 and low > 0:
		types += 1
		if types < max_types:
			percent = game.random_range(low,100)
			sp = int( (cp/10) * percent/100.0 )
			cp = cp - sp*10  
		else:
			sp = cp/10
			cp = 0
			
	# print pp	# print gp	# print sp	# print cp	# print "------"
	
	# Add the coins to the inventory
	coin_qty = [cp,sp,gp,pp]
	for c in [3,2,1,0]:
		if coin_qty[c] > 0:
			coin = container.item_find (7000+c) # check existing coin of this type
			if coin == OBJ_HANDLE_NULL:
				coin = create_item_in_inventory (7000+c, container)
				coin_qty_current = 0
			else:
				coin_qty_current = coin.obj_get_int (obj_f_money_quantity) 
			coin.obj_set_int (obj_f_money_quantity, coin_qty[c] + coin_qty_current)

def add_coins_exact (container, qty_add, type=3):
	coin = container.item_find (7000 + type)
	if coin == OBJ_HANDLE_NULL:
		coin = create_item_in_inventory (7000 + type, container)
		qty_current = 0
	else:
		qty_current = coin.obj_get_int(obj_f_money_quantity) 
	coin.obj_set_int (obj_f_money_quantity, qty_current + qty_add )

def monk(x,t=0):
	m = game.party[4]
	del_inv(m)
	add_coins(m,x,t)

#------------------------------------------------------------------------------
# Misc. Inventory functions
#------------------------------------------------------------------------------
def get_inv (obj, mode=0):

	slots = []
	if obj.type in [obj_t_npc, obj_t_pc]:
		if mode == 0:
			slots = range(0,16) + range(100,124)
		elif mode == 1:
			slots = range(0,16)
		elif mode == 2:
			slots = range(100,124)
		elif mode == 3:  # all except clothing 5, 10, 12
			slots = [0,1,2,3,4,6,7,8,9,11,13,14,15] + range(100,124)
		elif mode == 4:  # only clothing 5, 10, 12
			slots = [5,10,12]
	elif obj.type == obj_t_container:
		slots = range(100,300)  # was range(100,220)

	inv = []
	for s in slots:
		item = obj.item_worn_at(s)
		if item != OBJ_HANDLE_NULL:
			inv.append(item)
	return inv

def del_inv (container, mode=0):
	for item in get_inv(container, mode):
		item.destroy()
	if mode == 1:
		container.money_adj(container.money_get() * -1)

def add_to_inv_unique (obj, item):
	if not obj.has_item(descriptions[item]):
		add_to_inv(obj,item)

def xfer_inv (container,inv):
	for i in inv:
		container.item_get(i)

def inv_slots_open (npc):
	open = 0
	if npc.type in [obj_t_npc, obj_t_pc]:
		for s in range(100,124):
			item = npc.item_worn_at(s)
			if item == OBJ_HANDLE_NULL:
				open += 1
	return open

def inv_worth (npc):
	exclude = range(5000,5050) + range(7000,7004)
	inv = get_inv(npc)
	worth = 0
	for i in inv:
		if i.name not in exclude:
			worth += i.obj_get_int(obj_f_item_quantity) * i.obj_get_int(obj_f_item_worth)
	return worth

def devalue_inv (container, min=0, max=99999999):
	for item in get_inv(container):
		worth = item.obj_get_int(obj_f_item_worth)
		if worth in range(min, max):
			item.obj_set_int(obj_f_item_worth,0)

#------------------------------------------------------------------------------
# NPC scripting functions
#------------------------------------------------------------------------------
def san_dialog_special (attachee, triggerer, npcid=0, maps=0, start_line_adj=0, met_bit=0):

	if triggerer.type == obj_t_pc:

		if isinstance(maps,int):
			maps = [maps]

		# For unique NPCs, set "bit 0" of their global variable to test for having met.
		# This is so the record of having met will still hold if npc is a different mob.

		if attachee.leader_get() == OBJ_HANDLE_NULL:
			if attachee.map not in maps and maps[0] != 0:
				triggerer.begin_dialog (attachee, 900) 
			elif npcid > 0:
				if gbget(npcid,met_bit):
					triggerer.begin_dialog (attachee, 200 + start_line_adj)
				else:
					triggerer.begin_dialog (attachee, 100 + start_line_adj) 
					gbset(npcid,met_bit)
			elif attachee.has_met(triggerer):
				triggerer.begin_dialog (attachee, 200 + start_line_adj) 
			else:
				triggerer.begin_dialog (attachee, 100 + start_line_adj) 
		else:
			if attachee.d20_query_has_spell_condition(sp_Dominate_Person):
				triggerer.begin_dialog (attachee, 800) 
			elif attachee.d20_query_has_spell_condition(sp_Charm_Person):
				triggerer.begin_dialog (attachee, 700) 
			elif attachee.d20_query_has_spell_condition(sp_Charm_Monster):
				triggerer.begin_dialog (attachee, 700) 
			else: # recruited
				triggerer.begin_dialog (attachee, 400 + start_line_adj)

def san_enter_combat_special (attachee, triggerer, friends=2, friends_range=45):
	
	if attachee.leader_get() == OBJ_HANDLE_NULL:

		if attachee.obj_get_int(obj_f_critter_age) != 0:
			make_friends_join_combat (attachee, triggerer, friends, friends_range)

		# Recruits leave group if they match age of follower
		if attachee.obj_get_int (obj_f_critter_age) != 0:
			for npc in game.party:
				if attachee.obj_get_int (obj_f_critter_age) == npc.obj_get_int (obj_f_critter_age):
					triggerer.follower_remove(npc)
					npc.attack(triggerer)
					npc.float_line(20000,triggerer)

def san_start_combat_special (attachee, triggerer, friends=2, strategy=1, friends_range=0):

	# Stop guildhall combat, this should really be somewhere else
	if attachee.map == 5034 and game.global_vars[920] == 0:
		if find_npc_near_cone (attachee, 14627, 100):
			game.global_vars[920] = 1  # stop attempted
			stop_combat(attachee,1)
			ebon = create_obj (14724, game_leader().location)
			game_leader().begin_dialog (ebon, 4000)
			return 0
		
	if attachee.object_flags_get() & OF_CLICK_THROUGH:
		unconceal_particles(attachee)
		attachee.object_flag_unset(OF_CLICK_THROUGH)
		attachee.npc_flag_set(ONF_KOS)  # maybe not?

	attachee.item_wield_best_all()
	if attachee.has_feat(feat_two_weapon_fighting):
		if attachee.item_worn_at(4) == OBJ_HANDLE_NULL:
			attachee.item_wield_best_all()

	# Remove non combatants from combat
	if attachee.npc_flags_get() & ONF_NO_ATTACK:
		knock_out(attachee)
		attachee.npc_flag_unset(ONF_KOS)
		attachee.npc_flag_set(ONF_KOS_OVERRIDE)
		for pc in game.party:
			attachee.ai_shitlist_remove(pc)  # sets reaction to 50

	break_free (attachee)

	if attachee.leader_get() == OBJ_HANDLE_NULL:
		if attachee.obj_get_int(obj_f_critter_age) != 0:
			if friends_range == 0:
				friends_range = get_friends_range(attachee)  # returns 45, except for some maps
			make_friends_join_combat (attachee, triggerer, friends, friends_range)

	if strategy:
		set_strategy_target (attachee, strategy)

	return 0

def get_friends_range (attachee):
	if attachee.map == 5144:  # pirate cave
		return 70
	elif attachee.map == 5106:  # temple exterior
		return 75
	elif attachee.map == 5068:  # myrminmoss exterior, bugbear witch
		return 75
	elif attachee.map in (5066,5067):  # temple level 1, temple level 2
		return 99
	return 45

def san_dying_special (attachee, triggerer, npc_id=0, butcher=0):

	# Modifies the creature's CR if the party is over level 10.
	if should_modify_CR(attachee):
		modify_CR(attachee, get_av_level())

	if npc_id in range(500,700):

		# Mark the NPC as dead.
		game.global_vars[npc_id] = game.global_vars[npc_id] | 2

		# Update Bad Friend reputation.
		if attachee.leader_get() != OBJ_HANDLE_NULL:
			game.global_vars[29] = game.global_vars[29] + 1
			if game.global_vars[29] >= 2:
				game.party[0].reputation_add(6)  # Bad Friend reputation

	# Update Butcher of Hommlet reputation
	if butcher and attachee.leader_get() == OBJ_HANDLE_NULL:
		if game.global_vars[24] == 0:
			game.global_vars[23] = game.global_vars[23] + 1
		if game.global_vars[23] >= 2:
			game.party[0].reputation_add(1)

	# Destroy cheap equipment, to make looting easier
	if attachee.name not in (14498,14499,14500,8121,8122,8123,14178,14179,14180):  # drow
		if group_average_level(game.leader) > 5.0:
			for item in get_inv(attachee):
				if (
					 item.name in (6011,6045,6046,6449,6450,6451,6452,6453,6454)          # leather boots
					 or item.name in (6023,6202,6449)                                     # padded boots
					 or item.name in (6012,6046,6024,6448)                                # gloves
					 or item.name in (6233,6429,6124,6431,6433,6269,6428,6432,6430,6234)  # cloaks, bbroygbvgw
					 or item.name in (6434,  6235,6236,6478,6479)                         # hoodless circlet, bandanas
					):
					# item.item_flag_set(OIF_NO_LOOT)
					# item.item_flag_set(OIF_NO_DISPLAY)
					# item.item_flag_set(OIF_NO_TRANSFER)
					item.destroy()

	# Destroy npc buff items
	toggle_buff_items(attachee)
	
def san_resurrect_special (attachee, triggerer, npc_id=0):

	if npc_id in range(500,700):

		# Unmark the NPC as dead.
		if game.global_vars[npc_id] & 2:
			game.global_vars[npc_id] -= 2

		# Update Bad Friend reputation
		if attachee.leader_get() != OBJ_HANDLE_NULL:
			game.global_vars[29] = game.global_vars[29] - 1
			if game.global_vars[29] < 2:
				game.party[0].reputation_remove(6)

def first_join (attachee, triggerer, npc_id, match_speed=1):

	# Set items worth more than 5 gp non-transferable
	for item in get_inv(attachee):
		if item != OBJ_HANDLE_NULL and item.name not in range(7000,7004):
			if item.obj_get_int(obj_f_item_worth) > 500:
				item.item_flag_set(OIF_NO_TRANSFER)

	attachee.identify_all()

	if match_speed == 1:
		pc = game_leader()
		size_diff = attachee.get_size - pc.get_size
		speed = pc.obj_get_int(obj_f_speed_run)
		float_num(game_leader(),size_diff)
		if size_diff <= -2:
			speed = speed + 10000000
		if size_diff == -1:
			speed = speed + 5000000
		elif size_diff == 1:
			speed = speed - 5000000
		elif size_diff >= 2:
			speed = speed - 10000000
		attachee.obj_set_int (obj_f_speed_run, speed)

	gbset(npc_id,7)

#------------------------------------------------------------------------------
# Function: make_friends_join_combat()
#
# This brings a creature's nearby friends into combat, needed for two reasons:
#
#  1) Often, creatures with matching factions will not join combat, even if
#     they are only 10 feet away. Very annoying. This is seen in large
#     encounters areas like Temple Level 1 rooms with the Ogres, Bugbears,
#     and Gnolls. You kill half the room, then walk over to their friend and
#     they say "Good Day, Sir". Maybe the purpose of this is to prevent
#     overloading combat, and starting a KOS domino effect.
#
#  2) Creatures in an encounter with faction '0', and also do not have their
#     ONF_KOS set, will not join combat when their friend is attacked.
#     This is what you want, actually, since if they were truly friends they
#     would have the same faction. But sometimes you want some NPC's with 
#     faction '0' to be in a group without having to tweak all the factions.  
#
#  It works by pre-setting all creatures in the encounter to the same age,
#  using obj.obj_set_int(obj_f_critter_age). Then, when one creature in the
#  group enters combat, a call is made out to every creature withing 60 feet
#  with the same age to join combat, even if they are behind a wall in the
#  next room. This process extends outward every turn.
#
#  So, it creates a cascade effect which will find every creature with that
#  same age and sets their KOS and put them in to combat.
#
#  Caution, this needs to be used with care, and stay local for each unique
#  encounter group. If there are too many creatures with the same age value
#  spread around the map, the whole map can join combat and go KOS.
#  This must be avoided.
#
#  Create a unique age value for the encounter group, and give that same value
#  to each creature in the group, and don't reuse it anywhere else in the game.
#
#  For spawned groups:
#    Call the function new_age() to get a unique age value for that encounter.
#    On random maps, the age 103 is used.
#
#  For Mobs:
#    Set the age of each creature in the proto itself, record the value,
#    and don't reuse it.
#
#  Note, creatures with matching factions will still join in as usual.
#------------------------------------------------------------------------------

def make_friends_join_combat (attachee, triggerer, friends, friends_range):

	# Set KOS, otherwise they may attack one guy then stop
	if attachee.leader_get() == OBJ_HANDLE_NULL:
		attachee.npc_flag_set(ONF_KOS)

	# 0 = no friends, 1 = same proto, 2 = same age, 3 = same proto or same age
	friend_list = []

	# Add NPCs that match Proto Name to attachee's list of friends
	if friends in [1,3]:
		for npc in game.obj_list_cone (attachee, OLC_NPC, 100, 0, 360):
			if attachee.object_flags_get() & OF_DONTLIGHT or npc.object_flags_get() & OF_DONTLIGHT:
				continue
			if attachee.name == npc.name:
				friend_list.append(npc)

	# Add NPCs that match Age to attachee's list of friends
	if friends in [2,3]:
		for npc in game.obj_list_cone (attachee, OLC_NPC, 100, 0, 360):

			# Ignore the attempt to join combat, if either is OF_DONTLIGHT.
			if attachee.object_flags_get() & OF_DONTLIGHT or npc.object_flags_get() & OF_DONTLIGHT:
				continue

			# Forces NPC into the friends list, if both are OF_STONED.
			if attachee.object_flags_get() & OF_STONED and npc.object_flags_get() & OF_STONED:
				friend_list.append(npc)

			# Check if obj_f_critter_age match
			elif attachee.obj_get_int (obj_f_critter_age) == npc.obj_get_int (obj_f_critter_age):
				if attachee.obj_get_int (obj_f_critter_age) != 0:  # ignore those with no Age set
					friend_list.append(npc)

	# Call out to friends and make them join combat
	for npc in friend_list:
		if npc != attachee and npc.get_initiative() == 0:
			if npc.leader_get() == OBJ_HANDLE_NULL and not critter_is_unconscious(npc):
				float_mes(attachee,207,1)
				if npc.distance_to(attachee) <= friends_range:  # npc.has_los(attachee) is inconsistent, need 40 for druid grove
					float_mes(npc,208,1)
					npc.npc_flag_set(ONF_KOS)
					# Re-conceal. The attack command puts the npc into combat, but unconceals.
					if npc.critter_flags_get() & OCF_IS_CONCEALED:
						npc.attack(game_leader())
						npc.critter_flag_set(OCF_IS_CONCEALED)
						npc.fade_to(4,1,20)
					else:
						npc.attack(game_leader())
				else:
					float_mes(npc,222,1)
					float_num(npc,int(friends_range),1)
					float_num (npc, int(npc.distance_to(attachee)), 1)
					pass

#------------------------------------------------------------------------------
# Function: set_strategy_target()
#
# This function will set the target for an enemy creature in combat, based on
# it's current strategy. That target will be the one that is attacked when the
# strategy.tab is processed. It will also sometimes change the creature's
# strategy based on the state of combat.
#
# Finally, it can also set the strategy directly, if desired, but this function
# is not really meant to do that. The option is unused at this time.
#------------------------------------------------------------------------------

def set_strategy_target (attachee, strategy=1, set_strategy=0):
	
	# Get the name of the strategy
	if strategy == 1:
		strategy = get_strat_name (attachee.obj_get_int(obj_f_critter_strategy))
	else:
		strategy = strategy.lower()
	
	if set_strategy:
		set_strat (attachee, strategy)

	# Set exclusions
	if attachee.leader_get() == OBJ_HANDLE_NULL:

		# Roll if enemy attacker will Exclude NPCs in party as possible targets during combat.
		# Prevents PC from putting NPCs up front as fodder to take all the attacks.
		if getbit_1(attachee,10) == 0:
			npcbit_1(attachee,10)
			if game.random_range(0,1) == 1:
				npcbit_1(attachee,11)
		if getbit_1(attachee,11) == 1:
			exclusions = '1011101'  # exclude all NPCs, exclude unconscious, spiritual, otiluke
			float_mes(attachee,232,1)
		else:
			exclusions = '1010001'  # include all NPCS, exclude unconscious, spiritual, otiluke
			float_mes(attachee,233,1)

	else:  	# Note: AI won't let an NPC recruit attack an enemy with same faction.
		exclusions = '1010011'  # exclude unconscious, pc, spiritual, otiluke

	# Get list of enemies
	enemies = []

	if strategy == 'archer':  # default
		if can_see_party(attachee) == 0:
			set_strat(attachee,'archer2')  # identical to 'approach'
			strategy = 'archer2'
		enemies = group_list (attachee, 1, 0, exclusions)

	elif strategy == 'archer2':  # default with approach
		if can_see_party(attachee) == 1:
			set_strat(attachee,'archer')  # identical to 'default'
			strategy = 'archer'
		enemies = group_list (attachee, 1, 0, exclusions)

	elif strategy == 'berzerker':
		enemies = group_list (attachee, 1, 0, exclusions)

	elif strategy == 'coup de grace':  # will not work on targets with condition "unconscious", even if Held. 
		exclusions = '1111001' 
		if attachee.leader_get() != OBJ_HANDLE_NULL:
			exclusions = '1110011' 
		for enemy in group_list_radius (attachee, 1, 2, exclusions, 2.0):
			if enemy.d20_query(Q_Critter_Is_Held):
				enemies.append(enemy)

	elif strategy == 'coup de grace approach':
		exclusions = '1111001' 
		if attachee.leader_get() != OBJ_HANDLE_NULL:
			exclusions = '1110011' 
		for enemy in group_list_radius (attachee, 1, 2, exclusions, 30.0):
			if enemy.d20_query(Q_Critter_Is_Held):
				enemies.append(enemy)

	elif strategy == 'default':
		enemies = group_list (attachee, 1, 0, exclusions)

	elif strategy == 'defender':
		enemies = group_list (attachee, 1, 0, exclusions)

	elif strategy == 'flanker':
		enemies = []  # don't set target for flanker, let strategy.tab figure it out
			
	elif strategy == 'sniper':
		enemies = group_list (attachee, 1, 0, exclusions)

	dbug("\nattachee", str(attachee), "strategy")
	dbug("  enemies", str(enemies), "strategy")

	# Set target
	target = OBJ_HANDLE_NULL
	if enemies:
		target = enemies[0]  # closest enemy
		
		# Make Archer target a random enemy sometimes, for variety.
		if strategy == 'archer': 
			roll = 0
			if game.random_range(1,4) == 1:
				roll = game.random_range (0, len(enemies)-1)
				enemy = enemies[roll]
				if can_see_360 (attachee, enemy):
					if attachee.distance_to(enemy) < 60:
						target = enemy
			float_num(attachee,roll)
		
		# Attack target
		if target != OBJ_HANDLE_NULL:
			for obj in group_list (attachee, 2, 0, '0000000'):
				attachee.ai_shitlist_remove(obj)
			attachee.attack(target)

		# Debugs
		if target != OBJ_HANDLE_NULL:
			float_mes(target,231,1) # Melee Target
			dbug("  target", str(target), "strategy")
			pass
		else:
			dbug("  target = OBJ_HANDLE_NULL", -99, "strategy")
			pass
			
	s = attachee.obj_get_int(obj_f_critter_strategy)
	n = get_strat_name(s)
	dbug("  strategy", str(n), "strategy")
	
	return enemies

# Set Age of critter to the value of the mobs obj_f_secretdoor_effectname.
def set_age (attachee):
	age = attachee.obj_get_int(obj_f_secretdoor_effectname)
	if age:
		attachee.obj_set_int (obj_f_critter_age, age)
		if attachee.map not in (5151,5152):  # obj_f_secretdoor_effectname used for B1 wandering monsters
			attachee.obj_set_int (obj_f_secretdoor_effectname, 0)

# Gets a common Age, not used yet, to be used for each critter in a random encounter
def new_age():  
	if game.global_vars[40] < 200:
		game.global_vars[40] = 200
	else:
		game.global_vars[40] += 1
	return game.global_vars[40]

def can_see_party (npc):
	for pc in game.party:
		if can_see_360 (npc, pc):
			if not pc.is_unconscious():
				if not pc.d20_query_has_spell_condition (sp_Otilukes_Resilient_Sphere):
					return 1
	return 0

def can_see_360 (npc, obj):
	can = 0
	if npc.can_see(obj) or npc.has_los(obj):
		can = 1
	else:
		npc.rotation += 3.14
		if npc.can_see(obj) or npc.has_los(obj):
			can = 1
		npc.rotation += 3.14
	return can

def break_free (attachee):
	# NPC is Webbed or Entangled
	if attachee.d20_query(Q_Is_BreakFree_Possible):
		float_mes(attachee,225,2,4)
		# Was broken free at one point, so force an automatic break free
		if attachee.object_flags_get() & OF_UNUSED_40000:
			float_mes(attachee,226,2,4)
			strength = attachee.stat_base_get(stat_strength)
			attachee.stat_base_set(stat_strength, 50)
			attachee.d20_send_signal(S_BreakFree)
			attachee.stat_base_set(stat_strength, strength)
		# Have never been broken free, so make a legit break-free roll
		else:
			float_mes(attachee,227,2,4)
			attachee.d20_send_signal(S_BreakFree)
			if not attachee.d20_query(Q_Is_BreakFree_Possible):
				attachee.object_flag_set(OF_UNUSED_40000)

def can_five_foot_step (attachee, enemy):
	x1,y1 = location_to_axis(attachee.location)
	x2,y2 = location_to_axis(enemy.location)
	x = (x1+x2)/2
	y = (y1+y2)/2
	for obj in group_list (attachee, 2, 0, '1000000'):
		if obj in [attachee,enemy]:
			continue
		i,j = location_to_axis(obj.location)
		if i in range(x,x+2) and j in range(y,y+2):
			return 0
	return 1

def toggle_buff_items (attachee):
	buff_items = (
		6622,6623,6624,6625,      # shield of faith
		6626,6627,6628,6629,      # barkskin
		6617,6632,6633,           # mage armor, shield
		6634,6635,6636,6637,6638  # resist acid, cold, elec, fire, sonic
		)
	if attachee.leader_get() != OBJ_HANDLE_NULL or attachee.stat_level_get(stat_hp_current) <= -10:
		inv = get_inv(attachee)
		for i in inv:
			if i.name in buff_items:
				i.destroy()

def npc_weapon_change (attachee, items = [], options='10101'):
	options = int(options,2)
	if options & 1:  # remove weapons
		weapon_1 = attachee.item_worn_at(3)
		weapon_2 = attachee.item_worn_at(4)
		for weapon in [weapon_1, weapon_2]:
			if weapon != OBJ_HANDLE_NULL:
				weapon.destroy()
				if not options & 2:  # drop weapon
					game.obj_create (weapon.name, attachee.location)
	if options & 4:  # remove shield
		shield = attachee.item_worn_at(11)
		if shield != OBJ_HANDLE_NULL:
			shield.destroy()
			if not options & 8:  # drop shield
				game.obj_create (shield.name, attachee.location)
	if options & 16:  # add new weapons and/or shield
		for item in items:
			add_to_inv (attachee, item, 0, 0, 0, 0, 1)
	attachee.item_wield_best_all()

def restock (attachee, map=0, chest_proto=0, inv_src=0, self=0, clear=1, devalue=0, remove_duplicates=0):

	# Personal Equipment
	if self == 1:
		float_mes(attachee,221,3)
		create_inv (attachee, inv_src, clear)

	# Merchant Inventory
	else:
		float_mes(attachee,220,3)
		if attachee.map not in (map, 0):
			return
		box = find_container_near (attachee, chest_proto)
		if box != OBJ_HANDLE_NULL:
			create_inv (box, inv_src, clear)
			if chest_proto in range(1500,1699):
				gbset(chest_proto-1000,2)
			if remove_duplicates == 1:
				inv = get_inv(box)
				inv2 = []
				for i in inv:
					duplicate = 0
					for i2 in inv2:
						if (i.type != obj_t_scroll) and (i.name == i2.name):
							i.destroy()
							create_item_in_inventory(8037,box)
							duplicate = 1
							break
					if duplicate == 0:
						inv2.append(i)
			if devalue:
				for item in get_inv(box):
					item.obj_set_int(obj_f_item_worth,0)
	
def should_restock (glob, days=0):
	if time_since_restock(glob) > time_since_dawn() + (days * 86400):
		return 1
	return 0

def time_since_restock (glob):
	seconds_since_restock = game.time.time_game_in_seconds(game.time) - game.global_vars[glob]
	return seconds_since_restock

# Marks items in dealer's inventory, when their inventory is first created
def mark_inventory (attachee, chest):
	box = find_container_near (attachee, chest)
	if box != OBJ_HANDLE_NULL:
		for item in get_inv(box):
			item.item_flag_set(OIF_UBER)

# Removes items from inventory that were sold to dealer by PC
def clean_inventory (attachee, chest):
	box = find_container_near (attachee, chest)
	if box != OBJ_HANDLE_NULL:
		for item in get_inv(box):
			if not item.item_flags_get() & OIF_UBER:
				item.destroy()
				
def time_since_dawn():
	adj = -6
	if game.time.time_game_in_hours(game.time) in [0,1,2,3,4,5]:
		adj = 18
	seconds_since_dawn = (game.time.time_game_in_hours(game.time) + adj) * 3600
	seconds_since_dawn += game.time.time_game_in_minutes(game.time) * 60
	return seconds_since_dawn

def turn_off_at_night (attachee, map, day=0):
	if game.leader.map == map:
		if game.is_daytime():
			if attachee.object_flags_get() & OF_OFF:
				attachee.object_flag_unset(OF_OFF)
		else: # night
			if not (attachee.object_flags_get() & OF_OFF):
				attachee.object_flag_set(OF_OFF)
	else:
		if attachee.object_flags_get() & OF_OFF:
			attachee.object_flag_unset(OF_OFF)
	return

def turn_off_at_day (attachee, map, day=0):
	if game.leader.map == map:
		if game.is_daytime():
			if not (attachee.object_flags_get() & OF_OFF):
				attachee.object_flag_set(OF_OFF)
		else:
			if attachee.object_flags_get() & OF_OFF:
				attachee.object_flag_unset(OF_OFF)
	else:
		if attachee.object_flags_get() & OF_OFF:
			attachee.object_flag_unset(OF_OFF)
	return

def dawn():
	hour = game.time.time_game_in_hours(game.time)
	if hour in [0,1,2,3,4,5]:
		hour += 24
	elapse = 30 - hour
	game.fade(3600*elapse,0,0,0)
	
def gbget (npc, bit):
	if bit in range(0,31):
		if game.global_vars[npc] & 2**bit:
			return 1
	return 0

def gbset (npc, bit, start=0, end=0, reset=1):

	# Set a specific bit
	if start == 0 and end == 0:
		game.global_vars[npc] = game.global_vars[npc] | 2**bit 

	# Set a random available bit between the paramaters start and end
	else:
		if reset:  # reset the range of bits if all have been set
			if game.global_vars[npc] == 2**(end+1) - 2**start:
				game.global_vars[npc] = 0
		available = []
		for d in range(start,end+1):
			if not game.global_vars[npc] & 2**d:
				available.append(d)
		if available:
			roll = available[game.random_range(0,len(available)-1)]
			game.global_vars[npc] += 2**roll
			return roll

	return 0

def gbunset (npc, bit):
	if bit in range(0,31):
		if game.global_vars[npc] & 2**bit:
			game.global_vars[npc] = game.global_vars[npc] - 2**bit 

def gqs (quest, state=-1):
	if state in range(0,300):
		game.quests[quest].state = state
	return game.quests[quest].state

def ggf (g, v = 2):
	if v in [0,1]:
		game.global_flags[g] = v
	return game.global_flags[g]

def ggv (g, v = -99999): 
	if v != -99999:
		game.global_vars[g] = v
	return game.global_vars[g]

def ggvv (g, v = -99999): 
	if v != -99999:
		game.global_vars[g] = v
	return game.global_vars[g], int2bin(game.global_vars[g])

def int2bin(i):
	b = ""
	if i > 4294967295:
		b = "out of range"
	for e in range (30,-1,-1):
		if i & 2**e: 
			b += str("1")
		else:
			b += str("0")
	return b

# Return the first conscious pc in the group from left to right
def game_leader (skill=0):
	skills = (skill_bluff, skill_diplomacy, skill_intimidate, skill_sense_motive, skill_gather_information)
	pc, max = OBJ_HANDLE_NULL, -99
	if skill in skills:
		for obj in game.party:
			if obj.type == obj_t_pc:
				if not obj.is_unconscious():
					if obj.skill_level_get(skill) > max:
						pc = obj
						max = obj.skill_level_get(skill)
		return pc
	else:
		for obj in game.party:
			if obj.type == obj_t_pc:
				if not obj.is_unconscious():
					return obj
	return OBJ_HANDLE_NULL

def inv_src(attachee):
	return attachee.obj_get_int(obj_f_critter_inventory_source)

def shuffle_list (a_list):
	old_list = list(a_list)
	new_list = []
	for item in range(len(old_list)):
		roll = game.random_range(0,len(old_list)-1)
		new_list.append(old_list[roll])
		del old_list[roll]
	return new_list

def starting_equipment_cost():
	
	# Quick sloppy fix, to prevent Tosh merchants from offering starter equipment if any pc is level 2 or higher
	for pc in game.party:
		if pc.type == obj_t_pc and class_levels(pc) >= 2:
			return -1

	cost = 0
	for pc in game.party:
		if pc.leader_get() != OBJ_HANDLE_NULL:
			continue
		if (pc.stat_level_get(stat_level_barbarian)):
			cost += 7800
		elif (pc.stat_level_get(stat_level_bard)):
			cost += 8300
		elif (pc.stat_level_get(stat_level_cleric)):
			cost += 10800
		elif (pc.stat_level_get(stat_level_druid)):
			cost += 4300
		elif (pc.stat_level_get(stat_level_fighter)):
			cost += 11600
		elif (pc.stat_level_get(stat_level_monk)):
			cost += 700
		elif (pc.stat_level_get(stat_level_paladin)):
			cost += 11100
		elif (pc.stat_level_get(stat_level_ranger)):
			cost += 12800
		elif (pc.stat_level_get(stat_level_rogue)):
			cost += 7000
		elif (pc.stat_level_get(stat_level_sorcerer)):
			cost += 4600
		elif (pc.stat_level_get(stat_level_wizard)):
			cost += 4600
		elif (pc.stat_level_get(stat_level_favored_soul)): #Added by temple+
			cost += 7200  #Added by temple+
		elif (pc.stat_level_get(stat_level_scout)):  #Added by temple+
			cost += 4600  #Added by temple+
		elif (pc.stat_level_get(stat_level_warmage)):  #Added by temple+
			cost += 4600  #Added by temple+
		elif (pc.stat_level_get(stat_level_beguiler)):  #Added by temple+
			cost += 4600  #Added by temple+
		elif (pc.stat_level_get(stat_level_swashbuckler)):  #Added by temple+
			cost += 4600  #Added by temple+
	if cost < game.party[0].money_get():
		return cost
	return 0

def give_starting_equipment():

	# Pay for the equipment
	cost = starting_equipment_cost()
	game.party[0].money_adj(-cost)

	# Create the equipment
	for pc in game.party:
		if pc.leader_get() != OBJ_HANDLE_NULL:
			continue
		for pc_class in [7,8,9,10,11,12,13,14,15,16,17,34,46,47,48,49]: #Modified by Temple+ (Extended Base Classes)
			if pc.stat_level_get(pc_class) > 0:
				if pc_class < 18: #Modified by Temple+
					create_inv(pc, 993 + pc_class) #Modified by Temple+
				else: #Modified by Temple+
					create_inv(pc, 10000 + pc_class) #Modified by Temple+ (Extended Base Classes)
				game.particles("sp-Aid-END",pc)
				break

		# Temporarily remove the ranged weapon before equipping
		item_name = 0
		for slot in range(0,16) + range(100,124):
			item = pc.item_worn_at(slot)
			if item != OBJ_HANDLE_NULL:
				if item.obj_get_int(obj_f_weapon_range):
					item_name = item.name
					item.destroy()
					break
		pc.item_wield_best_all()
		if item_name:
			create_item_in_inventory(item_name, pc)

	# Effects
	game.sound(4705,2)  # cape swish
	game.sound(4404,1)  # pots and pans

def unconceal_particles (attachee):
	if attachee.name in (14262, 14870, 14885):  # troll, dire mon lizard, shambling mound
		game.particles("ef-splash",attachee)
		game.sound(4000,1)
	elif attachee.name == 14961:  # primordial shadow
		game.particles("sp-Aid-END",attachee)
		game.particles("sp-Chill Metal",attachee)
		game.sound(8402,1)  # spells\sp_detect_undead.wav
	elif attachee.name in (14289, 14861, 14941, 14968):  # shadow, spectre, greater shadow, lich
		game.particles("sp-call lightning-hit",attachee)
		game.particles("sp-call lightning-hit",attachee)
		game.sound(1450,1) 
		attachee.fade_to(255,1,1)  # spectre won't fade in properly from concealment. force it here.
	elif attachee.map == 5066:  # temple level 1
		game.particles("uc4",attachee)
		game.sound(4334,1)
		game.sound(4380,1)
	else:
		game.particles("mon-chicken-brown-hit",attachee)
		game.particles("mon-zombie-unconceal",attachee)

# Create an item in pc's inventory.
# If that pc's inventory is full, create in inventory of next pc that has space.
# If all pc's inventories are full, creates on the ground next to original pc.

def create_item_in_party (proto, pc, id=1):
	if len(get_inv(pc,2)) >= 24:
		for obj in game.party:
			if obj.type == obj_t_pc and obj != pc and len(get_inv(obj,2)) < 24:
				pc = obj
				break
	if isinstance (proto, int):
		item = create_item_in_inventory (proto, pc)
		if id == 1:
			item.item_flag_set(OIF_IDENTIFIED)
		return item
	elif isinstance (proto, str):
		add_to_inv (pc, proto)
		if id == 1:
			pc.identify_all()

def clean_ground (npc=OBJ_HANDLE_NULL, omit=[]):
	if npc == OBJ_HANDLE_NULL:
		npc = game_leader()
	olc_types = (OLC_ARMOR, OLC_WEAPON, OLC_FOOD, OLC_GENERIC, OLC_SCROLL, OLC_MONEY)
	for t in olc_types:
		for obj in game.obj_list_vicinity (npc.location, t):
			if obj.name not in omit:
				obj.destroy()

def switch_to_pc (npc, skill, line):
	pc = game_leader(skill)
	pc.turn_towards(npc)
	npc.turn_towards(pc)
	pc.begin_dialog(npc,line)

def closest_pc (npc):
	distance = 1000
	closest = OBJ_HANDLE_NULL
	for pc in game.party:
		if pc.type == obj_t_pc:
			if not pc.is_unconscious():
				if npc.distance_to(pc) < distance:
					distance = npc.distance_to(pc)
					closest = pc
	if closest == OBJ_HANDLE_NULL:  # just in case
		pc = game_leader()
	return closest

def closest_pc_distance (npc, include_npc=0):
	distance = 1000
	for pc in game.party:
		if include_npc == 1 or pc.type == obj_t_pc:
			if npc.distance_to(pc) < distance:
				distance = npc.distance_to(pc)
	return distance

def party_in_square (x1, x2, y1, y2, include_npc=1):
	for pc in game.party:
		if include_npc == 0 and pc.type != obj_t_pc:
			continue
		x,y = location_to_axis(pc.location)
		if (x not in range (x1,x2)) or (y not in range(y1,y2)):
			return 0
	return 1

def tele2 (dialer, town, x, y, line=1000):
	game.fade_and_teleport(300,0,0,town,x,y)
	game.timevent_add (bananaphone, (dialer, x, y, line), 100)

def bananaphone (dialer, x, y, line):
	operator = game.obj_create (14457, location_from_axis (x+1, y+1))
	dialer.begin_dialog (operator, line)


#------------------------------------------------------------------------------
# Map specific functions (should probably be in their own file)
#------------------------------------------------------------------------------

def at_trenn_camp (npc):
	if npc.map == 5052:
		x,y = location_to_axis(npc.location)
		if int(x) in range (375,440) and int(y) in range (360,420):
			return 1
	return 0

def at_rhondi_camp (npc):
	if npc.map == 5052:
		x,y = location_to_axis(npc.location)
		if int(x) in range (520,600) and int(y) in range (530,610):
			return 1
	return 0

# Check if unique NPC's on battlefield should run off from after war or peace.
# Commanders, Lieutenants, Sorcerer/Wizard/Cleric, Furt, also trenn wolves.
# Generic Soldiers have their own processing for this in their own script.

def runoff_if_war_or_peace (npc):
	if npc.map == 5052:
		if npc.leader_get() == OBJ_HANDLE_NULL and not npc.is_unconscious() and not game.combat_is_active():
			if game.global_vars[74] > 0:
				if at_trenn_camp (npc):
					if game.global_vars[74] in (1,4):
						npc.runoff (npc.location+15 +(4294967296*15))
					else:
						npc.runoff (npc.location+0 -(4294967296*15))
				elif at_rhondi_camp (npc):
					if game.global_vars[74] in (2,4):
						npc.runoff (npc.location-15 -(4294967296*7))
					else:
						npc.runoff (npc.location+10 +(4294967296*10))
				else:
					npc.runoff(npc.location-3)
				game.new_sid = 0
				return 1
	return 0

# Destroys Crossroads version in san_first_heartbeat, if their side lost on Battlefield.
# This is used on the regular Crossroads maps, not on the Battlefield.

def destroy_if_lost_battle (attachee, side):
	if attachee.map in [5054,5055,5056,5057,5061]:
		if attachee.leader_get() == OBJ_HANDLE_NULL:
			if ( (side == 1 and game.global_vars[74] in [2,3])      # trenn
			     or (side == 2 and game.global_vars[74] in [1,3])   # rhondi
			     or (side == 3 and game.global_vars[74] in [1,2,4]) # mercenaries
				):
				attachee.object_flag_set(OF_OFF)
				attachee.destroy()
				return 1
	return 0

def beneath():
	if game.story_state >= 3:
		if game_leader().reputation_has(25) == 0:  # Not archaic society member
			if game.quests[61].state >= qs_completed:  # glyphs for riss
				return 1
		if game_leader().reputation_has(25) == 1:  # Archaic society member
			if game.quests[120].state >= qs_completed:  # glyphs for mancy
				return 1
	return 0

def party_near_tower():  # Gelwea Tower
	for pc in game.party:
		if pc.type == obj_t_pc:
			x,y = location_to_axis(pc.location)
			if (x in range(440,480) and y in range(455,480)) == 0:
				return 0
	return 1

#------------------------------------------------------------------------------
# Returns the zone within Paladin's Cove where the object is located.
# The zone can be defined as a circle or a square.
#    - "circle" areas are defined by a focus and a radius.
#    - "square" areas are defined by two x,y points. The first point is the 
#       top-left corner, the second point is the bottom-right corner.
# The last value in the definition is the zone number, which will be returned
# if the object is contained within that zone.
#------------------------------------------------------------------------------
def cove_zone (object, particles=1):

	zones = ( ("circle",456,625,5.5,1),		# tower, harbor east
			("square",378,544,448,621,2),	# wall
			("circle",367,536,5.5,3),		# tower, behind inn
			("square",287,453,359,531,4),	# wall
			("circle",276,445,5.5,5),		# tower, east 
			("square",485,246,287,448,6),	# wall
			("circle",484,236,5.5,7),		# tower, north
			("square",497,247,556,311,8),	# wall
			("circle",563,315,5.5,9),		# tower, gate upper
			("square",574,324,581,337,10),	# wall
			("circle",589,341,5.5,11),		# tower, gate lower
			("square",600,350,685,440,12),	# wall
			("circle",692,444,5.5,13),		# tower, west
			("square",692,453,523,627,14),	# wall
			("circle",512,624,5.5,15)		# tower, harbor west
	)

	# turn on all particles the first time the function is called
	if game.global_flags[900] == 1 and particles == 1:
		cove_zone (object, 0)
		game.global_flags[900] = 0

	x,y = location_to_axis(object.location)
	
	for z in zones:
		if z[0] == "circle":
			if game.global_flags[900] == 1:
				for angle in range (0,50):
					angle = angle * 50 / 6.28
					x_loc = int(z[1] + cos(angle)*z[3])
					y_loc = int(z[2] + sin(angle)*z[3])
					game.particles('torch',location_from_axis(x_loc,y_loc))
				continue
			if (x - z[1])**2 + (y - z[2])**2 < z[3]**2:
				return z[4]
		elif z[0] == "square":
			y_diff = (z[4] - z[2]) / 2 # y2-y1/2
			x_diff = (z[3] - z[1]) / 2 # x2-x1/2
			h = y_diff + x_diff
			w = y_diff - x_diff
			for i in range(0,h+1):
				for j in range(0,w+1):
					if game.global_flags[900] == 1:
						game.particles('torch',location_from_axis(z[1]+i-j,z[2]+i+j))
						continue
					if (x == z[1]+i-j or x == z[1]+i-j+1) and y == z[2]+i+j:
						return z[5]
		
	return 0


# From san_start_combat_special, trying to stop combat

		# This code works for 1 round, but then they always jump back in.
		# The won't leave combat for good, so I had to use the above code.

		# attachee.remove_from_initiative()
		# attachee.ai_stop_attacking()
		# attachee.npc_flag_unset(ONF_KOS)
		# attachee.npc_flag_set(ONF_KOS_OVERRIDE)
		# for pc in game.party:
			# attachee.ai_shitlist_remove(pc)  # sets reaction to 50
		# game.update_combat_ui()


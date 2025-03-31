from toee import *
from utilities import *
from marc import *

###############################################################################
# Populates the given dungeon level with creatures and treasure.
###############################################################################
def spawn_dungeon (level, el_min, el_max, treasure_chance_default=33):

	# Read in area and encounter files
	areas = read_areas (level)
	encs, encs_keys = read_encs (level, el_max, el_min)
	encs_codes, encs_limits, encs_used = read_codes (encs)

	max_spots_in_area = (1, 5, 10, 16, 24)
	die_roll_in_area = ( (0,1,1), (1,4,1), (2,4,2), (2,6,4), (2,10,4) )
	game.global_vars[907], game.global_vars[908] = 0,0  # for dbug

	################################################################
	# Populate each area in the dungeon level with an encounter
	################################################################
	for area in areas:

		area_size, enc_size = area[0], area[0]
		age = new_age()  # create a unique age for this group
				
		# Roll if an encounter will exist in that area
		if game.random_range(1,100) > area[1]:
			float_mes(game.leader,400,5)
			dbug_spawn (level, area_size, enc_size, "NO ENCOUNTER")
			continue
		float_mes(game.leader,401,5)

		# Predetermined
		if game.random_range(1,100) <= area[2]:
			roll = game.random_range(0, len(area[3])-1)
			key = area[3][roll]
			enc = encs[key]
			float_mes(game.leader,402,5)

		# Random
		else:
		
			# Shrink encounter size to be smaller than area_size, just for variation.
			if area_size > 1 and game.random_range(1,100) <= 25:
				enc_size = game.random_range (1, area_size-1)

			roll_again, found = 1, 0

			# Roll an encounter.
			while roll_again > 0 and roll_again < 50:

				# Abort if there are no more possible encounters for this enc size.
				if len(encs_keys[enc_size]) == 0:
					float_mes(game.leader,415,5)
					dbug_spawn (level, area_size, enc_size, "NO ENCOUNTER")
					break

				roll = game.random_range(0, len(encs_keys[enc_size]) - 1)
				key = encs_keys[enc_size][roll]
				enc = encs[key]
				float_mes(game.leader,403,5);

				dbug("\nkey",key,"codes")
				dbug("  encs_used[key]",encs_used[key],"codes")
				dbug("  encs_limits[key]",encs_limits[key],"codes")

				# Check if this encounter has been used too many times.
				# If so, remove it from list of possible encounters, so it won't be rolled again.
				if encs_used[key] >= encs_limits[key]:
					dbug("\n    ** Encounter Use Limit Exceeded. Don't use this encounter now or again. **",-99,"codes")
					for size in range(0,5):
						if key in encs_keys[size]:
							encs_keys[size].remove(key)
					roll_again += 1
				else:
					encs_used[key] += 1
					roll_again, found = 0, 1
		
			if found == 0:
				continue

		float_mes(game.leader,405,5)
		float_num(game.leader,area_size,5)
		float_mes(game.leader,406,5)
		float_num(game.leader,enc_size,5)

		# Roll creature formation, and treasure chest spot
		formation, chest_spot = [], []
		if area[4]:
			roll = game.random_range(0, len(area[4]) - 1)
			formation = area[4][roll]

		treasure_chance = treasure_chance_default
		if area[5]:
			roll = game.random_range(0, len(area[5]) - 1)
			chest_spot = list(area[5][roll])
			if chest_spot[2] > 100:
				chest_spot[2] = chest_spot[2] - 100
				treasure_chance = 100 

		# Roll the quantity
		d = die_roll_in_area[enc_size]
		qty_roll = roll_dice (d[0], d[1], d[2])
			
		float_mes(game.leader,407,5)
		float_num(game.leader,qty_roll,5)
		dbug_spawn (level, area_size, enc_size, str(key))

		# Check if encounter is LARGE. This means the creatures themselves are large.
		if encs_codes[key].find("L") >= 0:
		
			# If spots for LARGE creature encounters exist for this area, use them.
			if area[6]: 
				roll = game.random_range(0, len(area[6]) - 1)
				formation = area[6][roll]
				# reduce area size by 1 because there are fewer LARGE spots defined for that area
				area_size = max (0, area_size-1)  # min=0
				
			# Reduce qty, if qty roll is greater than area size.
			if qty_roll > max_spots_in_area[area_size]:
				qty_roll = max_spots_in_area[area_size]
				float_num(game.leader,qty_roll,5)

		#######################################################################
		# 1. Create each group of monsters for the given encounter
		#######################################################################
		spots_used, spots_used_by_integers, spot_tracker = 0, 0, 0

		for group in enc[1:]:

			qty_specified = group[1]
 
			# spots_used can become greater than qty_roll, this is OK

			# Quantity specified is an positive integer, 1-25
			if qty_specified >= 1:
				qty_available = max (0, max_spots_in_area[area_size] - spots_used) 
				qty = int (qty_specified)
				qty = min (qty, qty_available)
				spots_used_by_integers += qty

			# Quantity specified is a negative integer, roll dice
			elif qty_specified < 0:
				qty_available = max (0, max_spots_in_area[area_size] - spots_used) 
				qty = game.random_range (1, qty_specified * -1)
				qty = min (qty, qty_available)
				spots_used_by_integers += qty

			# Quantity specified is a percentage, 0 - 0.99
			# Percentage is taken from the spots not used up by groups specified by integers
			elif qty_specified > 0:
				qty_available = max (0, qty_roll - spots_used_by_integers)
				qty = round (qty_available * qty_specified) 
				qty = int (qty)
				qty = min (qty, qty_roll - spots_used)  # fixes rounding errors

			# Quantity specified is '0', use 100% of remaining spots
			else:
				qty = max (0, qty_roll - spots_used)

			spots_used += qty

			float_mes(game.leader,408,5)
			float_num(game.leader,qty,5)

			# Create each individual monster in the group
			for creature in range(0, qty):
				spot = get_spot(area_size, enc_size, group[2], group[3], spot_tracker)
				if spot == -1:
					continue
				spot_tracker += 2**spot
				spot *= 3
				loc = location_from_axis(formation[spot], formation[spot+1])
				kos = group[4]
				if game.global_vars[901] & 2**0:  # TESTING - KOS OVERRIDE
					kos = 2
				obj = create_obj (group[0], loc, 0, kos) 
				obj.obj_set_int (obj_f_critter_age, age)
				obj.critter_flag_set (OCF_MUTE)
				rot = formation[spot+2]

				# Set concealment and Unset OF_KOS (if x,y has 100 added to it, clunky coding, sorry)
				if int(formation[spot+2]) in range (100,108):
					obj.critter_flag_set(OCF_IS_CONCEALED)
					obj.fade_to(4,1,10)
					obj.object_flag_set(OF_CLICK_THROUGH)
					obj.npc_flag_unset(ONF_KOS)
					rot = formation[spot+2] - 100

				# Unset OF_KOS (if x,y has 200 added to it, clunky coding, sorry)
				elif int(formation[spot+2]) in range (200,208):
					obj.npc_flag_unset(ONF_KOS)
					rot = formation[spot+2] - 200
	
				# Set Rotation
				if group[2] in [1,2]:  # formation or unorganized, use rotation
					obj.move(loc)
					obj.rotation = rot
				else:                  # scattered
					obj.move(loc)
					obj.rotation = game.random_range(0,62) * 0.10
				obj.obj_set_int (obj_f_critter_weight, int(obj.rotation*100.0))

		roll = game.random_range(1,100)
			
		######################################################################
		# 2. Create treasure chest with treasure equal to EL of the encounter
		######################################################################
		if (chest_spot) and (roll < treasure_chance):

			float_mes(game.leader,410,5)

			EL = enc[0][enc_size]
			chance_trapped = min(75,20+(3*EL))
			chance_locked = min(75,20+(5*EL))
			roll_trapped = game.random_range(1,100)
			roll_locked = game.random_range(1,100)

			# Roll the chest's proto, from a list of possible chests
			chests = [1002,1003,1004,1005,1006,1031,1046,1049]
			if len(chest_spot) > 3:  # chest list is directly specified  
				chests = chest_spot[3]
			proto = chests[game.random_range(0, len(chests)-1)]

			# Check if Trapped, and if so change the proto to a trapped chest.
			if game.random_range(1,100) < chance_trapped:
				chests_trapped = {1002:1300, 1003:1310, 1004:1320, 1005:1330, 1006:1340, 1031:1350, 1046:1360, 1049:1370}
				if proto in chests_trapped:
					proto = chests_trapped[proto] + game.random_range(0,min(6,EL))

			# Create the chest
			x,y,rot = chest_spot[0], chest_spot[1], chest_spot[2]
			loc = location_from_axis(x,y)
			chest = create_obj (proto, loc, -1)
			if level in (1,2):
				chest.obj_set_int (obj_f_container_key_id, level)
			chest.move(loc)
			chest.rotation = rot

			# Create the treasure
			add_to_inv (chest, 'CGI', EL, 1, 1, 0)

			# Check if Locked
			if game.random_range(1,100) < chance_locked:
				chest.container_flag_set(OCOF_LOCKED)
				lock_dc = game.random_range(1,EL*2) + 20
				chest.obj_set_int (obj_f_container_lock_dc, lock_dc)

			dbug_chests(level,EL,chance_trapped,chance_locked,proto)

	game.global_vars[38] = game.global_vars[38] | 2**level


#-----------------------------------------------------------------------------
# Gets the spot number where the creature will be created, based on:
#   area_size -  0,1,2,3 or 4. 
#   enc_size -  0,1,2,3 or 4.
#   organization -  0 = scatter, 1 = formation, 2 = unorganized
#   duty -  0 = boss in front, 1 = melee, 2 = ranged, 3 = boss in back.
#   spot_tracker -  marks which spots have been used already, by bit.
#-----------------------------------------------------------------------------
def get_spot (area_size, enc_size, organization, duty, spot_tracker):

	formations = {
		0: { # solo
			0: (0,0),	# boss in front
			1: (0,0),	# melee
			2: (0,0),	# ranged
			3: (0,0)	# boss in back
			},
		1: { # 5 spots (boss 2, melee 0-1, ranged 3, boss in back 4)
			0: (2,1,0,3,4),
			1: (0,1,2,3,4),
			2: (3,4,0,1,2),
			3: (4,3,0,1,2)
			},
		2: { # 10 spots (boss 4, melee 0-3, ranged 5-8, boss in back 9)
			0: (4,3,2,1,0,5,6,7,8,9),
			1: (0,1,2,3,4,5,6,7,8,9),
			2: (5,6,7,8,9,0,1,2,3,4),
			3: (9,8,7,6,5,0,1,2,3,4)
			},
		3: { # 16 spots (boss 7, melee 0-6, ranged 8-14, boss in back 15)
			0: (7,6,5,4,3,2,1,0,8,9,10,11,12,13,14,15),
			1: (0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15),
			2: (8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7),
			3: (15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7)
			},
		4: { # 24 spots (boss 11, melee 0-10, ranged 12-22, boss in back 23)
			0: (11,10,9,8,7,6,5,4,3,2,1,0,12,13,14,15,16,17,18,19,20,21,22,23),
			1: (0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23),
			2: (12,13,14,15,16,17,18,19,20,21,22,23,0,1,2,3,4,5,6,7,8,9,10,11),
			3: (23,22,21,20,19,18,17,16,15,14,13,12,0,1,2,3,4,5,6,7,8,9,10,11)
			}
		}

	max_spots_in_area = (1, 5, 10, 16, 24)
	spot = -1

	# Solo go on any random spot.
	if enc_size == 0:
		spot = game.random_range(0, max_spots_in_area[area_size]-1)

	# Formation goes on a specific unused spot based on duty.
	elif organization == 1:
		formation = formations[area_size][duty]
		for spot in formation:
			if not spot_tracker & 2**spot:
				break

	# Unorganized and Scattered goes on a random unused spot.
	elif organization == 2 or organization == 0:
		unused = []
		for s in range (0, max_spots_in_area[area_size]):
			if not spot_tracker & 2**s:
				unused.append(s)
		spot = unused[game.random_range(0, len(unused)-1)]

	return spot

#------------------------------------------------------------------------------
# Reads the area data in dungeon_areas.mes
#
# Arguments:
#   level - The dungeon level being populated.
#
# Returns:
#   areas - A list of every area where an encounter could be created.
#
#   area[0] = area size (0,1,2,3, or 4)
#   area[1] = chance of encounter (0-100)
#   area[2] = chance the encounter is predetermined (0-100)
#   area[3] = list of predetermined encounter keys to roll from
#   area[4] = list of possible creature spawn locations  
#   area[5] = list of possible treasure spawn locations 
#
#   Example format of an area in areas:
#
#   [4, 90, 25, ['goblin wardband','owlbears'], [(485,418,2.4, 481,418,2.8, 478,419,3.1, 488,419,1.7, 490,421,1.7, 476,421,3.5, 475,424,3.4, 491,424,1.5, 492,427,1.0, 475,427,3.8, 474,430,4.0, 492,430,1.0, 483,415,2.2, 479,416,3.0, 487,416,1.9, 491,418,1.6, 475,418,3.1, 473,422,3.5, 493,422,1.4, 494,425,1.2, 472,425,3.8, 471,418,2.3, 495,418,1.7, 483,411,2.2)], [(482,409,2.3), (488,404,4.0), (467,412,3.1), (497,437,0), (470,429,4.0)] ]
#
# See comments in dungeon_areas.txt for more detailed information.
#-----------------------------------------------------------------------------
def read_areas(level):
	f = file('data/scr/dungeon_areas.txt','r') #Modified by temple+
	lines = []
	for line in f.readlines():
		line = line.split('#')[0].strip()
		if line:
			lines.append(line)
	f.close()

	areas = []
	ptr = 0

	while ptr < len(lines):
	
		dungeon_level = int(lines[ptr])
		size = int(lines[ptr+1])
		chance = int(lines[ptr+2])
		predetermined = int(lines[ptr+3])
		ptr += 4

		encs = []
		if predetermined:
			for enc in lines[ptr].split(','):
				enc = enc.strip()
				if enc:
					encs.append(enc)
			ptr += 1

		spot_list_qty = int(lines[ptr])
		ptr += 1
		spot_lists = []
		for t in range (0, spot_list_qty):
			spot_lists.append(eval(lines[ptr+t]))
		ptr += spot_list_qty

		chest_list_qty = int(lines[ptr])
		ptr += 1
		chest_lists = []
		for t in range (0, chest_list_qty):
			chest_lists.append(eval(lines[ptr+t]))
		ptr += chest_list_qty

		spot_LARGE_list_qty = int(lines[ptr])
		ptr += 1
		spot_LARGE_lists = []
		for t in range (0, spot_LARGE_list_qty):
			spot_LARGE_lists.append(eval(lines[ptr+t]))
		ptr += spot_LARGE_list_qty

		if dungeon_level == level:
			area = []
			area.extend((size,chance,predetermined,encs,spot_lists,chest_lists,spot_LARGE_lists))
			areas.append(area)

	return areas

#-----------------------------------------------------------------------------
# Reads the encounter data from dungeon_areas.txt
#
# Arguments:
#   level - The dungeon level being spawned.
#   el_max - The maximim encounter level of encounters to include.
#   el_min - The minimum encounter level of encounters to include.
#
# Returns:
#   encs - A dictionary of all encounters in dungeon_areas.txt.
#   encs_keys - See below
#
# Example format of an enc in encs{}:
#
#     'goblin warband': ( (0,2,3,4,5), (14183,1,1,0,1), (14186,0.5,1,0,1), (14185,0.5,1,1,1) ),
#
#     enc[0] = encounter level of this encounter, for each area size
#     enc[1] = creature group information
#     enc[1][0] = proto of the creature
#     enc[1][1] = Number of creatures to be created for this proto
#          integer:
#            - Create this exact number of creatures. Range is 1-24.
#            - That many WILL be created, regardless of dice roll. Exception.
#            - That many WILL NOT be allowed to exceed the max number of spots
#              for the area size. The code will reduce the number in that case. 
#            - So if the number is 7, but the area size is 1 (only 5 spots),
#              then only 5 creatures will be created.
#          float:
#            - Create this % of the remaining spots rolled. Range is 0.01-0.99.
#          0:
#            - Create this creature for all remaining spots rolled.
#            - This will use up all remaining spots, rolled for that encounter,
#              so put that creature last on the list, and only use it once.
#            - This is mostly used for encounters with only one creature proto.
#     enc[1][2] = organization, 0=scatter, 1=formation, 2=unorganized formation
#     enc[1][3] = duty, 0=boss in front, 1=melee, 2=ranged, 3=boss in back
#     enc[1][4] = KOS, 0=default, 1=set ONF_KOS, 2=set ONF_KOS_OVERRIDE
#     enc[2].... same as enc[1]  
#     enc[3].... same as enc[1]  
#
# encs_keys[]: This a list of 5 lists.
#
# Each list has keys to the dictionary encs{} that are valid encounters for:
#  a. That area size, AND ALSO
#  b. The Encounter Level falls between el_max and el_min.
#
# They are separated like this so that a valid encounter can be rolled
# randomly the first time for a given area size without having to re-roll
# multiple times from encs{} until a valid one is found.
#
# The five area sizes are, in order:
#     0:	1 creature.
#     1:	1d4+1 creatures.	max 5	average 3.5
#     2:	2d4+2 creatures.	max 10	average 7
#     3:	2d6+4 creatures.  	max 16	average 11
#     4:	2d10+4 creatures.	max 24	average 15
#
# Example format of encs_keys[]:
#   [
#   ['gelatenous cube','wizard','yellow mold','ogre',minotaur'],
#   ['gargoyle','gnoll','kobold','dire rat','gnoll',bugbear'],
#   ['gnoll','goblin','bugbear','skeleton','zombie','ghoul'],
#   ['ghoul','goblin','bugbear','skeleton','zombie','ghoul'],
#   ['goblin','dire rat','centipede'],
#   ]
#
# See comments in dungeon_encs.txt for more detailed information.
#-----------------------------------------------------------------------------
def read_encs (level, el_max, el_min):
	f = file('data/scr/dungeon_encs.txt','r') #Modified by temple+
	lines = []
	for line in f.readlines():
		line = line.split('#')[0].strip()
		lines.append(line)
	lines.append('')
	f.close()

	args = { 's':0, 'f':1, 'u':2,		# organization
			 'b':0, 'm':1, 'r':2, 'w':3,	# duty
			 'd':0, 'k':1, 'o':2 }		# kos
	encs = {}
	encs_keys = [[],[],[],[],[]]
	ptr = 0

	while ptr < len(lines):

		if not lines[ptr]:
			ptr += 1
			continue

		name = lines[ptr]

		dungeon_levels = []
		for i in lines[ptr+1].replace(' ','').split(','):
			dungeon_levels.append(int(i))

		els_list = []
		line = lines[ptr+2].replace(' ','').split(',')
		for i in line:
			els_list.append(int(i))
		ptr += 3

		enc = []
		enc.append(els_list)

		while lines[ptr]:
			g = lines[ptr].replace(' ','').split(',')
			group = []
			group.extend( (int(g[0]), float(g[1]), args[g[2]], args[g[3]], args[g[4]]) )
			enc.append(group)
			ptr += 1

		if level in dungeon_levels:
			size = 0
			for el in els_list:
				if el and el >= el_min and el <= el_max :
					encs_keys[size].append(name)
				size += 1

		encs[name] = enc

	return encs, encs_keys

#--------------------------------------------------------------------------
# Reads codes from the end of the encounter name.
#
# This is a last minute sloppy fix to define more information about the
# encounter without having to redo the entire structure of the encounter
# dictionary. Extra information can now be added to the end of the
# encounter name defined in dungeon_encs.txt by using square brackets.
#
# It's currently used to define if the encounter needs LARGE spots, and
# also to track the max number of times that encounter can be used.
# If I think of other specifics I need to pass on, I'll tack them on.
#
# Examples:
#   "dragon blue [L1]"
#   "dragon green young [2]"
#   "fire toad [L]"
#
# Possible values within the brackets"
#   digit (1-9): the max number of times that encounter can be used. 
#   "L": the encounter is LARGE, meaning that the creatures themselves are
#        large, and they demand fewer spots which are more spread apart
#        when the encounter is spawned.
#
# Returns three dictionaries:
#   The key in each dictionary is the full encounter name string.
#	The value for each dictionary is:
#	  encs_codes: the string of the contents within the brackets.
#     encs_limits: the max number of times that encounter CAN be used.
#     encs_used: the number of times that encounter HAS been used.
#--------------------------------------------------------------------------
def read_codes (encs):

	encs_codes = {}
	encs_limits = {}
	encs_used = {}
	
	for k in encs:
		encs_codes[k] = ""
		if k.find("[") >= 0 and k.find("]") >= 0:
			encs_codes[k] = k.split('[')[1].split(']')[0].strip()
		encs_limits[k] = 2  # default
		for q in range(1,10):
			if str(q) in encs_codes[k]:
				encs_limits[k] = q
				break
		encs_used[k] = 0

	dbug("\nencs_codes = ", encs_codes, "codes"); 
	dbug("\nencs_limits = ", encs_limits, "codes")
	dbug("\nencs_used = ", encs_used, "codes")

	return encs_codes, encs_limits, encs_used

	
#--------------------------------------------------------------------------	
# Debugging code.
# Sends a crude list of spawned creatures to modules\ToEE\_dungeon_spawn.txt
#--------------------------------------------------------------------------

locs = {
	1: ("0. Entry", "1. Alcoves", "2. Kitchen", "3. Dining Room", "4. Lounge", "5. Zel Bedroom (6,7 too)", "8. Zel Workroom", "9. Zel Lab", "10. Storage Room", "11. Supply Room", "12. Library", "13. Portcullis Room Hall", "14. Auxiliary Storeroom", "15. Teleport Room North", "15. Teleport Rooms Hall (16 too)", "17. Smithy (18,19 too)", "20. Dead End Hallway", "20. Dead End Room", "21. Meeting Room", "22. Garden, left side", "22. Garden, main body", "23. Storage Room", "24. Mistress Room", "25. Rogahn's Bedroom", "26. Trophy Room", "27. Throne Room", "28. Worship Room", "29. Captains's Room", "29. Captain's Room 2", "30. Stairs", "31. Pool Room, North", "31. Pool Room, South", "32. Advisor Room", "33. Barracks", "35. Guest Room 1", "35. Guest Room 2", "35. Guest Room 3", "36. Empty Room Near Pit Trap", "37. Rec Room"),

	2: ("38. Access Room", "39. Museum", "41. Cavern", "43. Cavern", "44. Cavern", "46, 47: Cavern", "51. Side Cavern, Blue Walls", "52. Raised Cavern", "53. Grand Cavern of the Bats 1", "53. Grand Cavern of the Bats 2", "54. Treasure Cave nearby Solo", "55. Exit Cave", "56. Cavern of Statue", "56. Cavern of Statue nook Solo", "56. Cavern of Statue cave south"),

	11: ("earth temple", "earth temple upper left", "earth temple upper right", "bugbear office", "top left room level 1", "cell room", "cell 1", "cell 2", "cell 3", "cell 4", "torture chamber", "snake room", "picnic table room", "picnic table room south", "ghoul room", "pirate cell", "south hall - room 1", "south hall - room 2", "south hall - room 3", "south hall - room 4", "harpy room lower", "harpy room upper", "harpy ante 1", "harpy ante 2", "harpy ante 3", "throne room", "secret room", "top-right room", "top-right room 2", "spider room", "wonilon", "library", "romag office", "romag troops", "romag troops lower", "earth troop room", "big room right", "big room left", "vapor rat room", "small room 1", "small room 2", "bugbear 3-door room", "south stairs alcove", "hall - stairs up SW", "hall - stairs up SE", "hall - by ghoul room SW", "hall - by bugbear guard room W", "hall - by bugbear guard room NW", "hall - long hall to stairs NW", "hall - off earth temple NW", "hall - stairs down to level 2", "hall - off earth temple NE", "hall - south of library", "hall - skeleton hall", "hall - south of two big rooms"),

	21: ("0. Entry", "1. Alcoves", "2. Kitchen", "3. Dining Room", "4. Lounge", "5. Zel Bedroom (6,7 too)", "8. Zel Workroom", "9. Zel Lab", "10. Storage Room", "11. Supply Room", "12. Library", "13. Portcullis Room Hall", "14. Auxiliary Storeroom", "15. Teleport Room North", "15. Teleport Rooms Hall (16 too)", "17. Smithy (18,19 too)", "20. Dead End Hallway", "20. Dead End Room", "21. Meeting Room", "22. Garden, left side", "22. Garden, main body", "23. Storage Room", "24. Mistress Room", "25. Rogahn's Bedroom", "26. Trophy Room", "27. Throne Room", "28. Worship Room", "29. Captains's Room", "29. Captain's Room 2", "30. Stairs", "31. Pool Room, North", "31. Pool Room, South", "32. Advisor Room", "33. Barracks", "35. Guest Room 1", "35. Guest Room 2", "35. Guest Room 3", "36. Empty Room Near Pit Trap", "37. Rec Room", "37 A. Maze Area Cave"),

	22: ("38. Access Room", "39. Museum", "41. Cavern", "42. Webbed Cave", "42 A. Webbed Cave, South", "43. Cavern", "44. Cavern", "45. Mystical Stone", "46, 47: Cavern", "48. Arena Cavern", "50. Water Pit", "51. Side Cavern, Blue Walls", "52. Raised Cavern", "53. Grand Cavern of the Bats 1", "53. Grand Cavern of the Bats 2", "53. Grand Cavern of the Bats 3", "54. Treasure Cave nearby Solo", "55. Exit Cave", "56. Cavern of Statue", "56. Cavern of Statue nook Solo", "56. Cavern of Statue cave south"),

	}

def dbug_spawn (level, area_size, enc_size, enc_name):
	if game.global_vars[909] == 1:
		if level in (1,2,11,21,22):
			if game.global_vars[908] == 0:
				dbug("\nLevel " + str(level) + ", Spawned creatures.", -99, "spawn")
				dbug("--------------------------------------------------------------------", -99, "spawn")
				pass
			spacer = ""
			for i in range (0, 32 - len(locs[level][game.global_vars[908]])):
				spacer = spacer + " "
			dbug(str(area_size) + "  " + str(enc_size) + "  " + locs[level][game.global_vars[908]] + spacer + enc_name , -99, "spawn")
			game.global_vars[908] += 1

def dbug_chests (level, EL, chance_trapped, chance_locked, proto):
	if game.global_vars[909] == 1:
		if level in (1,2,11,21,22):
			if game.global_vars[907] == 0:
				dbug("\nLevel " + str(level) + ", Spawned chests.", -99, "chests")
				dbug("--------------------------------------------------------------------", -99, "chests")
				pass
			spacer = ""
			for i in range (0, 32 - len(locs[level][game.global_vars[908]-1])):
				spacer = spacer + " "
			dbug(locs[level][game.global_vars[908]-1] + spacer + "   EL = " + str(EL) + ", chance_trapped = " + str(chance_trapped) + ", chance_locked = " + str(chance_locked) + ", Chest = " + str(proto), -99, "chests")
			game.global_vars[907] = 1


# TESTING, Create all chests, put this before chests are created.

		# # TESTING - CREATE ALL CHESTS
		# if game.global_vars[901] & 2**1:
			# for c in area[5]:
				# loc = location_from_axis(c[0], c[1])
				# proto = chest_list[game.random_range(0, len(chest_list)-1)]
				# chest = create_obj (proto, loc, -1)
				# chest.move(loc)
				# chest.rotation = c[2]
			# continue


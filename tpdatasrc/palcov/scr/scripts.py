from toee import *
from utilities import *

#------------------------------------------------------------------------------
# Originally in scripts.py
#------------------------------------------------------------------------------

# def npc_1(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_3, 1 )
	# return

# def npc_2(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_4, 1 )
	# return

# def npc_3(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_5, 1 )
	# return

# def npc_1_undo(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_3, 0 )
	# return

# def npc_2_undo(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_4, 0 )
	# return

# def npc_3_undo(attachee):
	# attachee.obj_set_int( obj_f_npc_pad_i_5, 0 )
	# return

# def get_1(attachee):
	# x1 = attachee.obj_get_int( obj_f_npc_pad_i_3 )
	# if x1 != 0:
		# return 1
	# return

# def get_2(attachee):
	# x2 = attachee.obj_get_int( obj_f_npc_pad_i_4 )
	# if x2 != 0:
		# return 1
	# return

# def get_3(attachee):
	# x3 = attachee.obj_get_int( obj_f_npc_pad_i_5 )
	# if x3 != 0:
		# return 1
	# return

def npcvar_1(attachee, var):
	attachee.obj_set_int( obj_f_npc_pad_i_3, var )
	return

def npcvar_2(attachee, var):
	attachee.obj_set_int( obj_f_npc_pad_i_4, var )
	return

def npcvar_3(attachee, var):
	attachee.obj_set_int( obj_f_npc_pad_i_5, var )
	return

def getvar_1(attachee):
	x1 = attachee.obj_get_int( obj_f_npc_pad_i_3 )
	if x1 != 0:
		return x1
	return 0

def getvar_2(attachee):
	x2 = attachee.obj_get_int( obj_f_npc_pad_i_4 )
	if x2 != 0:
		return x2
	return 0

def getvar_3(attachee):
	x3 = attachee.obj_get_int( obj_f_npc_pad_i_5 )
	if x3 != 0:
		return x3
	return 0

# Reserved bits in bit_1 for any generic npc.

# bit 0     - personal inventory created.
# bit 2     - container inventory created, used for guards spawning container treasure
# bit 4     - weapons created in san_enter_combat.
# bit 6     - buff spell chance for wizard, roll has been performed.
# bit 7     - buff spell chance for wizard, rolled Yes.
# bit 10    - roll made deciding if NPC attacker will exclude NPC's in party as possible targets during combat.
# bit 11    - NPC attacker will exclude NPC's in party as possible targets during combat.
# bit 20-30 - unique to each NPC. Farm Girl at Farm Random, Devil has cast fear, etc.

def npcbit_1 (attachee, bit, set=1):
	x1 = getvar_1(attachee)
	mask = 2**bit
	if set == 1:
		x1 = x1 | mask
	elif x1 & mask:
		x1 = x1 - mask
	npcvar_1(attachee, x1)
	return

def npcbit_2 (attachee, bit, set=1):
	x2 = getvar_2(attachee)
	mask = 2**bit
	if set == 1:
		x2 = x2 | mask
	elif x2 & mask:
		x2 = x2 - mask
	npcvar_2(attachee, x2)
	return

def npcbit_3 (attachee, bit, set=1):
	x3 = getvar_3(attachee)
	mask = 2**bit
	if set == 1:
		x3 = x3 | mask
	elif x3 & mask:
		x3 = x3 - mask
	npcvar_3(attachee, x3)
	return

def getbit_1 (attachee, bit):
	if attachee.obj_get_int(obj_f_npc_pad_i_3) & (2**bit):
		return 1
	return 0
		
def getbit_2 (attachee, bit):
	if attachee.obj_get_int(obj_f_npc_pad_i_4) & (2**bit):
		return 1
	return 0
		
def getbit_3 (attachee, bit):
	if attachee.obj_get_int(obj_f_npc_pad_i_5) & (2**bit):
		return 1
	return 0

#------------------------------------------------------------------------------
# Originally in Co8.py
#------------------------------------------------------------------------------

def readMes(mesfile):
	""" Read the mesfile into a dictionary indexed by line number. """
	mesFile = file(mesfile,'r')
	mesDict = {}
	for line in mesFile.readlines():
		# Remove whitespace.
		line = line.strip()
		# Ignore empty lines.
		if not line:
			continue
		# Ignore comment lines.
		if line[0] != '{':
			continue
		# Decode the line.  Just standard python string processing.
		line = line.split('}')[:-1]
		for i in range(len(line)):
			line[i] = line[i].strip()
			line[i] = line[i][1:]
		contents = line[1:]
		# Insert the line into the mesDict.
		mesDict[int(line[0])] = contents
	mesFile.close()
	# print 'File read'
	return mesDict

def writeMes(mesfile, mesDict):
	""" Write the dictionary mesDict as a mesfile.  This does not presever comments (although it could if desired).  Overwrites mesfile if it already exists."""
	mesFile = file(mesfile, 'w')
	linenumbers = mesDict.keys()
	linenumbers.sort()
	for linenumber in linenumbers:
		mesFile.write('{'+str(linenumber)+'}{')
		for thing in mesDict[linenumber]:
			if thing == mesDict[linenumber][-1]:
				mesFile.write(str(thing))
			else:
				mesFile.write(str(thing)+', ')
		mesFile.write('}\n')
	mesFile.close()
	# print 'File written'
	return 1

###################################################################
#Added by Darmagon                                                #
###################################################################
SPELL_FLAGS_BASE = obj_f_secretdoor_dc
OSF_IS_ANTIDOTE = 1           # used in Antitoxin
OSF_IS_SUMMONED = 128         # used in GetCritterHandle, familiars, warp wood
OSF_IS_HEZROU_STENCH = 256    # used in stench.py
ITEM_HOLDER = 1027            # proto 1027   

def set_spell_flag( obj, flag):
	val = obj.obj_get_int(SPELL_FLAGS_BASE)
	obj.obj_set_int(SPELL_FLAGS_BASE, val | flag)
	return obj.obj_get_int(SPELL_FLAGS_BASE)

def get_spell_flags(obj):
	return obj.obj_get_int(SPELL_FLAGS_BASE)

def is_spell_flag_set(obj, flag):
	return obj.obj_get_int(SPELL_FLAGS_BASE) & flag

def unset_spell_flag(obj, flag):
	val = obj.obj_get_int(SPELL_FLAGS_BASE)
	if val & flag:
		obj.obj_set_int(SPELL_FLAGS_BASE, val-flag)
	return obj.obj_get_int(SPELL_FLAGS_BASE)

# currently used in Antidote, and Delayed Blast Fireball, marc
def find_spell_obj_with_flag( target,item, flag ):
	ret = OBJ_HANDLE_NULL
	item_holder = game.obj_create(ITEM_HOLDER, target.location)
	prot_item = target.item_find_by_proto(item)

	while prot_item != OBJ_HANDLE_NULL and ret == OBJ_HANDLE_NULL:
		if is_spell_flag_set(prot_item, flag)!= 0:
			ret = prot_item
		prot_item.item_flag_unset(OIF_NO_DROP)
		item_holder.item_get(prot_item)
		prot_item = target.item_find_by_proto(item)
	prot_item = item_holder.item_find_by_proto(item)

	while prot_item!= OBJ_HANDLE_NULL:
		target.item_get(prot_item)
		prot_item.item_flag_set(OIF_NO_DROP)
		prot_item = item_holder.item_find_by_proto(item)
	item_holder.destroy()
	return ret

# currently used in Anitdote, marc
def destroy_spell_obj_with_flag(target, proto_id, flag):
	ret = 0
	item_holder = game.obj_create(ITEM_HOLDER, target.location)
	prot_item = target.item_find_by_proto(proto_id)

	while prot_item != OBJ_HANDLE_NULL:
		if is_spell_flag_set(prot_item, flag)!= 0:
			ret = 1
			print "found it"
			break
		prot_item.item_flag_unset(OIF_NO_DROP)
		item_holder.item_get(prot_item)
		prot_item = target.item_find_by_proto(proto_id)

	if ret == 1:
		prot_item.destroy()
		print "destroyed it"

	prot_item = item_holder.item_find_by_proto(proto_id)

	while prot_item!= OBJ_HANDLE_NULL:
		target.item_get(prot_item)
		if proto_id == 6271:
			prot_item.item_flag_set(OIF_NO_DROP)
		prot_item = item_holder.item_find_by_proto(proto_id)
	item_holder.destroy()
	return ret

#################################################################
# Added by Cerulean the Blue                                    #
#################################################################
def End_Spell(spell):                           # marc, used in Raise Dead, Reincarnate, Extraplanar Chest
	spell.summon_monsters( 1, 14460 )
	critter = GetCritterHandle( spell, 14460)
	spell.caster.follower_remove(critter)
	critter.destroy()
	return 

def GetCritterHandle( spell, critter_name ):    # marc, used in Summon Familiar, Mord. Faithful Hound
	# Returns a handle that can be used to manipulate the summoned  creature object
	for critter in game.obj_list_vicinity( spell.target_loc, OLC_CRITTERS ):
		if (critter.name == critter_name and not (is_spell_flag_set(critter, OSF_IS_SUMMONED)) and critter.is_friendly(spell.caster)):
			set_spell_flag( critter, OSF_IS_SUMMONED)
			return critter
	return OBJ_HANDLE_NULL

def Timed_Destroy(obj, time):                   # marc, used in Extraplanar Chest, Mord. Faithful Hound
	game.timevent_add( destroy, ( obj ), time) # 1000 = 1 second
	return

def destroy(obj): # Destroys object.  Necessary for time event destruction to work.
	obj.destroy()
	return 1


#------------------------------------------------------------------------------
# Originally in combat_standard_routines.py
#------------------------------------------------------------------------------

def should_modify_CR( attachee ):
	# uses npc_get flag # 31
	party_av_level = get_av_level()
	if party_av_level > 10 and npc_get(attachee, 31) == 0:
		return 1
	else:
		return 0
	
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


#------------------------------------------------------------------------------
# Originally in py00439script_daemon.py
#------------------------------------------------------------------------------

#########################################
# Bitwise NPC internal flags		#
# 1-31					#
# Uses obj_f_npc_pad_i_4		#
# obj_f_pad_i_3 is sometimes nonzero    #
# pad_i_4, pad_i_5 tested clean on all  #
# protos				#
#########################################

def npc_set(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	tempp = attachee.obj_get_int(obj_f_npc_pad_i_4) | abc
	attachee.obj_set_int(obj_f_npc_pad_i_4, tempp)
	return	

def npc_get(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	return attachee.obj_get_int(obj_f_npc_pad_i_4) & abc != 0


#------------------------------------------------------------------------------
# Originally in InventoryRespawn.py
#------------------------------------------------------------------------------

def CalcGJ(string, value):

	gem_table = {
		1: [10, [12043, 12044]],	# Rhodochrosite, Malachite
		2: [50, [12041, 12042]],	# Carnelian, Blue Jasper
		3: [100, [12035, 12040]],	# White Pearl, Amber
		4: [500, [12034, 12039]],	# Black Pearl, Aquamarine
		5: [1000, [12010, 12038]],	# Emerald, Blue Sapphire
		6: [5000, [12036, 12037]]	# Diamond, Jacinth
		} 

	jewelry_table = {
		1: [50, [6180, 6190]],		# Plain Copper Ring, Tribal Necklace
		2: [100, [6181, 6185]],		# Silver Ring, Bronze Ring
		3: [200, [6157]],			# Gold Chain
		4: [250, [6182, 6194]],		# Gold Ring, Silver Necklace
		5: [500, [6186, 6191]],		# Platinum Ring, Heavy Gold Chain
		6: [750, [6183, 6193]],		# Garnet Ring, Scarab Necklace
		7: [1000, [6184, 6192]],	# Jasper Ring, Jade Pendant
		8: [2500, [6187, 6197]],	# Fancy Gold Ring, Silver Medallion Necklace
		9: [5000, [6188, 6195]],	# Ruby Ring, Fancy Gold Chain
		10: [7500, [6189, 6196]]	# Sapphire Ring, Jade Necklace
		}

	gjlist = []
	if string == 'gems':
		table = gem_table
	elif string == 'jewelry':
		table = jewelry_table
	else:
		return gjlist
	if not (type(value) is int):
		value = ConvertToInt(value)
		if not (type(value) is int):
			return gjlist
	n = len(table)
	while value >= table[1][0]:
		if table[n][0] <= value:
			gjlist.append(table[n][1][game.random_range(0, len(table[n][1]) - 1)])
			value = value - table[n][0]
		else:
			n = n - 1
	return gjlist

def ConvertToInt( string ):
	if type(string) is str:
		try:
			string = int(string)
		except:
			if not (string == 'gems' or string == 'jewelry'):
				print 'WARNING: NON-INTEGER FOUND'
				print 'Non-integer found is', string
	else:
		print 'WARNING:  NON-STRING FOUND'
		print 'Non-string found is', string
	return string


#------------------------------------------------------------------------------
# Originally in SummonMonsterTools.py
#------------------------------------------------------------------------------

def SummonMonster_Rectify_Initiative(spell, proto_id):
	monster_obj = SummonMonster_GetHandle(spell, proto_id)
	
	if monster_obj != OBJ_HANDLE_NULL:
		SummonMonster_Set_ID(monster_obj, game.random_range(1, 2**30) )
		caster_init_value = spell.caster.get_initiative()
		monster_obj.set_initiative( caster_init_value - 1 )
		game.update_combat_ui()
	return

def SummonMonster_GetHandle( spell, proto_id ):
# Returns a handle that can be used to manipulate the familiar creature object
	for obj in game.obj_list_vicinity( spell.target_loc, OLC_CRITTERS ):
		stl = spell.target_loc
		stlx, stly = location_to_axis(stl)
		ox, oy = location_to_axis(obj.location)
		if (obj.name == proto_id) and ( (ox-stlx)**2 + (oy-stly)**2 ) <= 25:
			if not ( SummonMonster_Get_ID( obj ) ):
				return obj
	return OBJ_HANDLE_NULL

def SummonMonster_Get_ID(obj):
# Returns embedded ID number
	return obj.obj_get_int(obj_f_secretdoor_dc)

def SummonMonster_Set_ID( obj, val ):
# Embeds ID number into mobile object.  Returns ID number.
	obj.obj_set_int( obj_f_secretdoor_dc, val )
	return obj.obj_get_int( obj_f_secretdoor_dc )
	
def SummonMonster_Clear_ID( obj ):
# Clears embedded ID number from mobile object
	obj.obj_set_int( obj_f_secretdoor_dc, 0 )

def get_options_from_mes(teststr):
	print "get_options_from_mes"
	options = []
	i_file = open('data\\mes\\spells_radial_menu_options.mes','r')
	s = i_file.readline()
	while s != teststr and s !='':
		s = i_file.readline()
	s = i_file.readline()
	str_list = s.split()
	num_str = str_list[1]
	num_str = num_str.strip()
	num_str = num_str.replace("{","")
	num_str = num_str.replace("}","")
	num_options = int(num_str)
	i = 0
	while i < num_options:
		s = i_file.readline()
		str_list = s.split()
		num_str = str_list[1]
		num_str = num_str.strip()
		num_str = num_str.replace("{","")
		num_str = num_str.replace("}","")
		options.append(int(num_str))
		i = i + 1
	i_file.close()
	return options


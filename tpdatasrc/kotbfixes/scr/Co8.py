from toee import *

from utilities import *

from ed import *

from batch import *



###################################################################
###  (18:55 20/04/06) A script written by Glen Wheeler (Ugignadl) for manipulating ToEE files.
###  Requested by Cerulean the Blue from Co8. 
##
###  (13:05 22/04/06) Added a few functions which should enable the implementation of a bag of
### holding.  Some of them use the already written functions here (just import this scripts as a library).
##
##
##	(08/1/2013) Added Co8 configuration options (Sitra Achara)
##	
###################################################################


def boh_newbag(bagcontents=[11300], bagnum=0, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This function only takes keyword arguments.  bagnum of 0 will ask the function to use the lowest available, otherwise will use that number.  If it already exists the game will crash (deliberately, I can change that behaviour).  bgfilename is the filename for all the bags.  The contents should be a list (or tuple) of integers.   Also, if you want each BOH to start with something inside them here is where you can do it (say with a note on how to use them, about the charged things being broken and crafted stuff etc). """
	#  So we read in all the bags.
	allBagDict = readMes(bgfilename)
	#  Then insert the new bag.
	linenumbers = allBagDict.keys()
	if bagnum:
		if bagnum in allBagDict:
			raise 'BadbagnumError'
	else:
		bagnum = 1
		while bagnum in linenumbers:
			bagnum += 1
	
	allBagDict[bagnum] = bagcontents
	#  Then write it all back again.
	# writeMes(bgfilename, allBagDict)
	# print 'New Bag'
	return bagnum

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


def boh_getContents(bagnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This can be called when opening the bag.  The bag filename is (I am assuming) fixed, but can be set by passing a keyword argument.  bagnum is the line number to use for that bag in the bgfilename. """
	allBagDict = readMes(bgfilename)
	#  Note:  If bagnum is not an integer we will have a CTD.  I can fix this if it is the case by coercing the datatype.
	contents = __boh_decode(allBagDict[bagnum])
	#  contents will be an ordered list of integers, ready to be turned into objects.
	# print ' Get Contents'
	return contents

def __boh_decode(bohcontents):
	""" bohcontents should just be a comma delimited series of integers.  We don't have any more support than that (CB: if we have more complex data types in the bag in future then this is the function to be changed.)   Sub-bags (i.e. BOH within BOH) could be implemented by decoding lists within lists.  However I'm not even sure that is in accordance with the rules. """
	# [0] is there to comply with the output of the readMes function.
	l = bohcontents[0].split(', ')
	for i in range(len(l)):
		#  This should cause a crash, but I am testing with non-integers.  If you want to be hardcore then just remove the try--except.
		try:
			l[i] = int(l[i])
		except:
			print "WARNING: NON-INTEGER FOUND IN BAG OF HOLDING!"
			print 'Non-integer found is', l[i]
	# print 'Decoded'
	return l

def _boh_removeitem(bagnum, itemnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" Remove the item itemnum from the bag bagnum.  If it's not in there we get a (deliberate) crash. """
	allBagDict = readMes(bgfilename)
	contents.remove(itemnum)
	allBagDict[bagnum] = contents
	writeMes(bgfilename, allBagDict)
    
def _boh_insertitem(bagnum, itemnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This function will insert the integer itemnum at the end of the list associated with bagnum in bgfilename. """
	allBagDict = readMes(bgfilename)
	contents.append(itemnum)
	allBagDict[bagnum] = contents
	writeMes(bgfilename, allBagDict)

###################################################################
#Added by Darmagon	                                              #
###################################################################
SPELL_FLAGS_BASE = obj_f_secretdoor_dc
has_obj_list = []
objs_to_destroy_list = []
active_spells = []
holder = OBJ_HANDLE_NULL
holder2 = OBJ_HANDLE_NULL
OSF_IS_IRON_BODY = 1
OSF_IS_TENSERS_TRANSFORMATION = 2
OSF_IS_ANALYZE_DWEOMER = 4		## also used by ShiningTed for Antidote
OSF_IS_HOLY_SWORD = 8			## also used by ShiningTed for Foresight
OSF_IS_PROTECTION_FROM_SPELLS = 16
OSF_IS_MORDENKAINENS_SWORD = 32
OSF_IS_FLAMING_SPHERE = 64
OSF_IS_SUMMONED = 128
OSF_IS_HEZROU_STENCH = 256
OSF_IS_TONGUES = 512
OSF_IS_DISGUISE_SELF = 1024
OSF_IS_DEATH_WARD = 2048
ITEM_HOLDER = 1027
	
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

def are_spell_flags_null(obj):
	if obj.obj_get_int(SPELL_FLAGS_BASE) == 0:
		return 1
	return 0
	
def check_for_protection_from_spells(t_list, check):
	global has_obj_list
	global objs_to_destroy_list	
	global holder
	global holder2
	holder = game.obj_create(14629, game.party[0].location)
	holder2 = game.obj_create(14629, game.party[0].location)
	ret = 0
	
	for obj in t_list:
		prot_obj = obj.obj.item_find_by_proto(6400)
		
		while prot_obj != OBJ_HANDLE_NULL and is_spell_flag_set(prot_obj, OSF_IS_PROTECTION_FROM_SPELLS)==0:
			prot_obj.item_flag_unset(OIF_NO_DROP)
			holder.item_get(prot_obj)
			prot_obj = obj.obj.item_find_by_proto(6400)
		get_back_obj = holder.item_find_by_proto(6400)
		while get_back_obj != OBJ_HANDLE_NULL:
			obj.obj.item_get(get_back_obj)
			get_back_obj.item_flag_set(OIF_NO_DROP)
			get_back_obj = holder.item_find_by_proto(6400)
		if prot_obj != OBJ_HANDLE_NULL:
			ret = 1
			has_obj_list.append(obj.obj)
			prot_obj.item_flag_unset(OIF_NO_DROP)
			holder2.item_get(prot_obj)		
						
			new_obj = game.obj_create(6400,game.party[0].location)
			set_spell_flag(new_obj, OSF_IS_PROTECTION_FROM_SPELLS)			
			objs_to_destroy_list.append(new_obj)			
							
			new_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 0, 8)
			new_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 1, 8)
			new_obj.item_condition_add_with_args('Saving Throw Resistance Bonus', 2, 8)
			obj.obj.item_get(new_obj)
	holder.destroy()
	if ret == 0:
		holder2.destroy()
	return ret

def	replace_protection_from_spells():
	global has_obj_list
	global objs_to_destroy_list
	global holder2
	for obj in objs_to_destroy_list:
		obj.destroy()
	objs_to_destroy = []
	count = 0
	while count < len(has_obj_list):
		obj_to_get = holder2.item_find_by_proto(6400)
		has_obj_list[count].item_get(obj_to_get)
		obj_to_get.item_flag_set(OIF_NO_DROP)
		count = count + 1
	holder_objs = []
	has_obj_list= []
	holder2.destroy()
		
class obj_holder:
	obj = OBJ_HANDLE_NULL
	def __init__(self, obj):
		self.obj = obj

class spell_with_objs_packet:
	def __init__(self, spell_obj, sentinel1, sentinel2, spell):
		self.spell_obj = spell_obj
		self.sentinel1 = sentinel1
		self.sentinel2 = sentinel2
		self.spell = spell
		
def is_obj_in_active_spell_list(obj):
	ret = None
	for object in active_spells:
		if obj == object.sentinel1 or obj == object.sentinel2 or obj == object.spell_obj:
			return object
	return ret

def get_active_spells():
	return active_spells
	
def append_to_active_spells(spell, spell_obj, sent1, sent2):
	global active_spells
	new_spell = spell_with_objs_packet(spell_obj,sent1,sent2,spell)
	ofile = open("append.txt", "w")
	ofile.write(str(new_spell.sentinel1) + "\n")	
	active_spells.append(new_spell)
	ofile.write(str(game.time.time_in_game())+ "\n")
	ofile.write(str(get_active_spells()) + "\n")
	ofile.close()

def remove_from_active_spells(spell_packet):
	global active_spells
	count = 0
	while count < len(active_spells):
		if active_spells[count] == spell_packet:
			active_spells[count].sentinel1.destroy()
			active_spells[count].sentinel2.destroy()
			del active_spells[count]
			break
		count = count + 1	

def build_obj_list(list):
	t_list = []
	for t in list:
		t_list.append(obj_holder(t))
	return t_list
	
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
		if proto_id == 6400:
			prot_item.item_flag_set(OIF_NO_DROP)
		prot_item = item_holder.item_find_by_proto(proto_id)
	item_holder.destroy()
	return ret
	
	
def set_blast_delay(num):
	#if num >= 0 and num <=5:
	#	ifile = open("delayed_blast_fireball.txt", "w")
	#	ifile.write(str(num))
	#	ifile.close()
	print "someone actually uses set_blast_delay?\n"
	return # was this even used?
	
def is_in_party(obj):
	for x in game.party:
		if obj == x:
			return 1
	return 0

def unequipD( slot, npc, whole_party = 0):	# edited by Ted so as not to interfere with the other one
	unequip_set = []
	if whole_party:
		unequip_set = game.party
	else:
		unequip_set = [npc]
	for npc2 in unequip_set:
		i = 0
		j = 0
		item = npc2.item_worn_at(slot)
		if item != OBJ_HANDLE_NULL:
			if item.item_flags_get() & OIF_NO_DROP:
				item.item_flag_unset(OIF_NO_DROP)
				i = 1
			if item.item_flags_get() & OIF_NO_TRANSFER:
				item.item_flag_unset(OIF_NO_TRANSFER)
				j = 1
			holder = game.obj_create(1004, npc2.location) 
			holder.item_get(item)
			tempp = npc2.item_get(item)
			pc_index = 0
			while tempp == 0 and pc_index < len(game.party): #this part is insurance against filled up inventory for any PCs
				if game.party[pc_index].type == obj_t_pc:
					tempp = game.party[pc_index].item_get(item)
				pc_index += 1
			if i:
				item.item_flag_set(OIF_NO_DROP)
			if j:
				item.item_flag_set(OIF_NO_TRANSFER)
			holder.destroy()
#	game.particles( "sp-summon monster I", game.party[0] )
	# item = npc.item_worn_at(slot)
	# holder = game.obj_create(1004, npc.location) 
	# holder.item_get(item)
	# npc.item_get(item)
	# holder.destroy()


#################################################################
#End added by Darmagon                                          #
#################################################################

#################################################################
#Updated by Shiningted 19/9/9                                   #
#################################################################

def weap_too_big(weap_user):
	if weap_user.is_category_type( mc_type_giant ):
		return
	weap_1 = weap_user.item_worn_at(3)
	weap_2 = weap_user.item_worn_at(4)
	size_1 = weap_1.obj_get_int(obj_f_size)
	if size_1 > STAT_SIZE_MEDIUM and weap_2 != OBJ_HANDLE_NULL:
		unequipD( 3, weap_user)
	if weap_2 != OBJ_HANDLE_NULL: # fix - added OBJ_HANDLE_NULL check
		size_2 = weap_2.obj_get_int(obj_f_size)
		if size_2 > STAT_SIZE_MEDIUM:
			unequipD( 4, weap_user)
	return

#################################################################
#End added by Shiningted                                        #
#################################################################

#################################################################
# Added by Cerulean the Blue                                    #
#################################################################

def read_field( object, field ):
	return object.obj_get_int( field )

def write_field( object, field, value ):
	object.obj_set_int( field, value )
	return object.obj_get_int( field )
	
def clear_field( object, field ):
	object.obj_set_int( field, 0 )
	return object.obj_get_int( field )

def GetCritterHandle( spell, critter_name ):
	# Returns a handle that can be used to manipulate the summoned  creature object
	for critter in game.obj_list_vicinity( spell.target_loc, OLC_CRITTERS ):
		if (critter.name == critter_name and not (is_spell_flag_set(critter, OSF_IS_SUMMONED)) and critter.is_friendly(spell.caster)):
			set_spell_flag( critter, OSF_IS_SUMMONED)
			return critter
	return OBJ_HANDLE_NULL

def End_Spell(spell):
	spell.summon_monsters( 1, 14456 )
	critter = GetCritterHandle( spell, 14456)
	spell.caster.follower_remove(critter)
	critter.destroy()
	return 

def Timed_Destroy(obj, time):
	game.timevent_add( destroy, ( obj ), time) # 1000 = 1 second
	return
	
def Timed_Runoff(obj, runoff_time = 1000, runoff_location = -1):
	if runoff_location == -1:
		obj.runoff(obj.location-3)
	elif type(runoff_location) == type( obj.location ):
		obj.runoff(runoff_location)
	elif type(runoff_location) == type( [ 1 , 2 ] ):
		obj.runoff(     location_from_axis(runoff_location[0], runoff_location[1])        )
	else:
		obj.runoff(obj.location-3)
	
	game.timevent_add( Timed_Runoff_Set_OF_OFF, ( obj ), runoff_time) # 1000 = 1 second, default
	
	return

def Timed_Runoff_Set_OF_OFF( obj ):
	obj.object_flag_set(OF_OFF)
	return


def destroy(obj): # Destroys object.  Neccessary for time event destruction to work.
	obj.destroy()
	return 1

def StopCombat(obj, flag):
	if type(obj) == type(OBJ_HANDLE_NULL.location):
		loc1 = obj
	else:
		loc1 = obj.location
	# game.particles( 'Orb-Summon-Air-Elemental', game.party[0] )
	for pc in game.party:
		for critter in game.obj_list_vicinity( loc1, OLC_CRITTERS ):
			critter.ai_shitlist_remove( pc )
			if flag == 1:
				critter.npc_flag_unset(ONF_KOS)
			critter.reaction_set( pc, 40 )
			critter.ai_shitlist_remove( pc )
			pc.ai_shitlist_remove(critter)
			critter.reaction_set( pc, 40 )
		if type(obj) == type(pc):
			obj.ai_shitlist_remove(pc)
	return 

def group_challenge_rating():
	return (group_average_level(game.leader) * (len(game.party)/4.0))


#################################################################
# Added by Hazelnut                                             #
#################################################################

# Replacement for the D20STD_F_POISON flag for saving throws. The STD define contains
#	the enum index value, 4, which is incorrect as it's checked against the bitmask 8
#	in temple.dll.
D20CO8_F_POISON = 8

# Util functions for getting & setting words, bytes and nibbles in object integers.
# object = reference to the object containing the integer variable.
# var	 = the variable to be used. e.g. obj_f_weapon_pad_i_2
# idx	 = the index of the word (0-1), byte (0-3) or nibble (0-7) to use.
# val	 = the value to be set.

def getObjVarDWord(object, var):
	return object.obj_get_int(var)

def setObjVarDWord(object, var, val):
	object.obj_set_int(var, val)

def getObjVarWord(object, var, idx):
	return getObjVar(object, var, idx, 0xffff, 16)

def setObjVarWord(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xffff, 16)

def getObjVarByte(object, var, idx):
	return getObjVar(object, var, idx, 0xff, 8)

def setObjVarByte(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xff, 8)

def getObjVarNibble(object, var, idx):
	return getObjVar(object, var, idx, 0xf, 4)

def setObjVarNibble(object, var, idx, val):
	setObjVar(object, var, idx, val, 0xf, 4)

def getObjVar(object, var, idx, mask, bits):
	bitMask = mask << (idx * bits)
	val = object.obj_get_int(var) & bitMask
	val = val >> (idx * bits)
	return (val & mask)

def setObjVar(object, var, idx, val, mask, bits):
	#print "obj=", object, " var=", var, " idx=", idx, " val=", val
	bitMask = mask << (idx * bits)
	val = val << (idx * bits)
	oldVal = object.obj_get_int(var) & ~bitMask
	object.obj_set_int(var, oldVal | val)

##################
# added by dolio #
##################

# Temporarily renders a target invincible, and deals
# some damage (which gets reduced to 0). This allows
# the dealer to gain experience for killing the target.
def plink( critter, spell ):
	invuln = critter.object_flags_get() & OF_INVULNERABLE

	dice = dice_new( '1d1' )

	critter.object_flag_set( OF_INVULNERABLE )

	critter.spell_damage( spell.caster,
			      D20DT_UNSPECIFIED,
			      dice,
			      D20DAP_UNSPECIFIED,
			      D20A_CAST_SPELL,
			      spell.id )

	if not invuln:
		critter.object_flag_unset( OF_INVULNERABLE )

def slay_critter( critter, spell ):
	plink( critter, spell )

	critter.critter_kill()

def slay_critter_by_effect( critter, spell ):
	plink( critter, spell )

	critter.critter_kill_by_effect()

# This kills a critter, and gets rid of the body, while
# dropping the equipment.
def disintegrate_critter( ensure_exp, critter, spell ):
	spell.duration = 0

	if ensure_exp:
		plink( critter, spell )

	critter.critter_kill()

	critter.condition_add_with_args( 'sp-Animate Dead',
					 spell.id, spell.duration, 3 )

# end dolio additions

#################################################################
# Added by Sitra Achara                                         #
#################################################################



def config_override_bits( var_no , bit_range , new_value ):
	if type(bit_range) == type( [1,2,3] ):
		bit_maskk = 0
		for diggitt in bit_range:
			bit_maskk += 2**diggitt
		game.global_vars[var_no] ^=  ( bit_maskk & ( game.global_vars[var_no] ^ ( new_value << bit_range[0] ) ) )
	elif type(bit_range) == type( 1 ):
		if new_value != 0:
			game.global_vars[var_no] |= (2**bit_range)
		else:
			game.global_vars[var_no] -= ( game.global_vars[var_no] & (2**bit_range) )
		
	
def get_Co8_options_from_ini():
	try:
		i_file = open('Co8_config.ini','r') # opens the file in ToEE main folder now 

		s = 'initial value'
		failsafe_count = 0
		while s != '' and failsafe_count < 1000:
			failsafe_count += 1
			s = i_file.readline()
			s2 = s.split('{')
			if len(s2) >= 3: 
				# check if it's an actual entry line with the format:
				# { Param name } { value } {description (this bracket is optional!) } 
				# will return an array ['','[Param name text]} ','[value]}, [Description text]}'] entry

				param_name = s2[1].replace("}","").strip()
				par_lower = param_name.lower()
				param_value = s2[2].replace("}","").strip()
				
				if param_value.isdigit():
				
					param_value = int(param_value)
					
					if par_lower == 'Party_Run_Speed'.lower():
						config_override_bits( 449, range(0, 2+1), param_value )

					elif par_lower == 'Disable_Sheating_Weapons'.lower():
						config_override_bits( 450, 1, param_value )	
						
					elif par_lower == 'Disable_New_Plots'.lower():
						config_override_bits( 450, 0, param_value )
					elif par_lower == 'Disable_Target_Of_Revenge'.lower():
						config_override_bits( 450, 10, param_value )
					elif par_lower == 'Disable_Moathouse_Ambush'.lower():
						config_override_bits( 450, 11, param_value )
					elif par_lower == 'Disable_Arena_Of_Heroes'.lower():
						config_override_bits( 450, 12, param_value )
					elif par_lower == 'Disable_Reactive_Temple'.lower():
						config_override_bits( 450, 13, param_value )
								
					elif par_lower == 'Entangle_Outdoors_Only'.lower():
						config_override_bits( 451, 0, param_value )
					elif par_lower == 'Elemental_Spells_At_Elemental_Nodes'.lower():
						config_override_bits( 451, 1, param_value )
					elif par_lower == 'Charm_Spell_DC_Modifier'.lower():
						config_override_bits( 451, 2, param_value )
					elif par_lower == 'AI_Ignore_Summons'.lower():
						config_override_bits( 451, 3, param_value )
					elif par_lower == 'AI_Ignore_Spiritual_Weapons'.lower():
						config_override_bits( 451, 4, param_value )
					elif par_lower == 'Random_Encounter_XP_Reduction'.lower():
						config_override_bits( 451, range(5, 7+1), param_value )  # bits 5-7
					elif par_lower == 'Stinking_Cloud_Duration_Nerf'.lower():
						config_override_bits( 451, 8, param_value )
						
		if game.global_vars[449] & (2**0 + 2**1 + 2**2) != 0:
			speedup( game.global_vars[449] & (2**0 + 2**1 + 2**2) , game.global_vars[449] & (2**0 + 2**1 + 2**2) )
		i_file.close()
	except:
		return
	return
	
	
	
def alter_tiles( work_dir, file_list, source_dir):
	
	qq = 0
	while qq < len( file_list ):
		
		f_name = file_list[qq]
		
		# NB: work_dir and source_dir have to end in a /
		if source_dir[ len(source_dir) - 1] == '/' and work_dir[ len(work_dir) - 1] == '/':
			
			f_source = open('//modules/ToEE/' + source_dir + f_name, 'rb')
			
			temp = f_source.read()
			
			# temptell = f_source.tell()
			
			ff = open('//modules/ToEE/' + work_dir + f_name, 'wb')
			
			ff.write(temp)
			
			ff.close()
			
			f_source.close()
			
		else:
			print 'error! missing \\ in folder definition for file swap' 
		qq += 1
	
from toee import *
from __main__ import game
import _include ## Neccessary for timedEventAdd to work across saves and loads - CtB

# function: party_transfer_to( target, oname )
# arguments: target - PyObjHandle (the recepient of the item)
#            oname - the internal name of the object to be trasnfered
#                    from the party
# returns: the PyObjHandle of the object found, OBJ_HANDLE_NULL if not found
def party_transfer_to( target, oname ):
	for pc in game.party:
		item = pc.item_find( oname )
		if item != OBJ_HANDLE_NULL:
			pc.item_transfer_to( target, oname )
			return item
	return OBJ_HANDLE_NULL

def party_destroy( oname ):
	for pc in game.party:
		item = pc.item_find( oname )
		if item != OBJ_HANDLE_NULL:
			item.destroy()
	return OBJ_HANDLE_NULL

def find_npc_near( obj, name ):  # range is 45
	for npc in game.obj_list_vicinity( obj.location, OLC_NPC ):
		if (npc.name == name):
			return npc
	return OBJ_HANDLE_NULL

def find_npc_near_cone( obj, name, range=60 ):  # default range is 60
	for npc in game.obj_list_cone (obj, OLC_NPC, range, 0, 359):
		if (npc.name == name):
			return npc
	return OBJ_HANDLE_NULL

def find_container_near( obj, name ):
	for container in game.obj_list_vicinity( obj.location, OLC_CONTAINER ):
		if (container.name == name):
			return container
	return OBJ_HANDLE_NULL

def group_average_level( pc ):

	# 1. Find the party member with the highest HD.
	highest = 0
	for obj in pc.group_list():
		if obj.hit_dice_num > highest:
			highest = obj.hit_dice_num

	# 2. Add up total party levels, ignoring those of 4 HD less than highest 
	total_hd, total_members = 0, 0
	for obj in pc.group_list():
		if obj.hit_dice_num >= highest - 4:
			total_hd += obj.hit_dice_num
			total_members += 1

	# 3. Compute the average
	avg = float(total_hd) / float(total_members)

	# 4. Adjust +15%/-15% for each valid party member more/less than 4
	# Still debating this number. Could be anywhere from 5% to 20%
	adjustment = avg * 0.15 * (total_members - 4)
	avg = avg + adjustment
	avg = max (avg,1)

#	count = 0
#	level = 0
#	for obj in pc.group_list():
#		count = count + 1
#		level = level + obj.stat_level_get( stat_level )
#	if (count == 0):
#		return 1
#	#level = level + ( count / 2 )
#	#avg = level / count
#	avg = (level/count) + (count - 4)/2
	return avg

def group_wilderness_lore( pc ):
	high = 0
	level = 0
	for obj in pc.group_list():
		level = obj.skill_level_get( skill_wilderness_lore )
		if (level > high):
			high = level
	if (high == 0):
		return 1
	return high

def obj_percent_hp( obj ):
	curr = obj.stat_level_get( stat_hp_current )
	max = obj.stat_level_get( stat_hp_max )
	if (max == 0):
		return 100
	if (curr > max):
		curr = max
	if (curr < 0):
		curr = 0
	percent = (curr * 100) / max
	return percent

def group_percent_hp( pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		percent = percent + obj_percent_hp(obj)
		cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent

def group_pc_percent_hp( attachee,pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		if (obj != attachee):
			percent = percent + obj_percent_hp(obj)
			cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent

def group_kobort_percent_hp( attachee,pc ):
	percent = 0
	cnt = 0
	for obj in pc.group_list():
		if (obj != attachee and obj.name != 8005):
			percent = percent + obj_percent_hp(obj)
			cnt = cnt + 1
	if (cnt == 0):
		percent = 100
	elif (percent < 0):
		percent = 0
	else:
		percent = percent / cnt
	return percent


def create_item_in_inventory( item_proto_num, npc ):
	item = game.obj_create(item_proto_num, npc.location)
	if (item != OBJ_HANDLE_NULL):
#		if item.obj_get_int(obj_f_item_worth) > 1000:
#			item.item_flag_set(OIF_NO_PICKPOCKET)
		npc.item_get(item)
	return item

def is_daytime():
	return game.is_daytime()

def is_safe_to_talk(speaker,listener):
	if (speaker.can_see(listener)):
		if (speaker.distance_to(listener) <= 15):
			return 1
	return 0

def can_talk (npc):
	if not game.combat_is_active():
		if not npc.object_flags_get() & OF_OFF:
			if npc.leader_get() == OBJ_HANDLE_NULL:
				if not npc.npc_flags_get() & ONF_KOS:
					if not npc.is_unconscious():
						return 1
	return 0

def start_game_with_quest(quest_number):
	game.quests[quest_number].state = qs_accepted
	# hommlet movies start at 1009 and go to 1017
	# but the quests start at 22, so offset by 987
	game.fade_and_teleport(0,0,987 + quest_number,5001,711,521)
	return

def start_game_with_botched_quest(quest_number):
	game.quests[quest_number].state = qs_mentioned
	game.quests[quest_number].state = qs_botched
	# hommlet no-voice movies start at 1018 and go to 1026
	# but the quests start at 22, so offset by 996
	game.fade_and_teleport(0,0,996 + quest_number,5001,711,521)
	return

def critter_is_unconscious(npc):
	curr = npc.stat_level_get( stat_hp_current )
	if (curr < 0):
		return 1
	if (npc.stat_level_get(stat_subdual_damage) > curr):
		return 1
	return 0

# HTN - returns true if obj is an "item" (obj.h)
def obj_is_item( obj ):

	return (obj.type == obj_t_projectile) or (obj.type == obj_t_weapon) or (obj.type == obj_t_ammo) or (obj.type == obj_t_armor) or (obj.type == obj_t_scroll) or (obj.type == obj_t_bag)

# HTN - compares two OBJ_HANDLE's by "hit_dice", used for sorting OBJ_HANDLE lists
def obj_comparison_by_hit_dice( obj_1, obj_2 ):

	obj_1_hitdice = obj_1.hit_dice
	obj_2_hitdice = obj_2.hit_dice

	return cmp( obj_1_hitdice, obj_2_hitdice )

def pyspell_targetarray_copy_to_obj_list( spell ):
	copy_list = []
	for item in spell.target_list:
		copy_list.append( item.obj )
	return copy_list

# Function: location_to_axis( PyLong loc )
# Author  : Steve Moret
# Returns : A tuple of two PyInts
# Purpose : returns an x, y for a specified location
#           can be used like:  
#  x, y = location_to_axis( npc.location )
#
def location_to_axis( loc ):
	if type(loc) == type(OBJ_HANDLE_NULL):
		loc = loc.location
		# in case the object was given as an argument instead of its location
	y = loc >> 32
	x = loc & 4294967295
	return ( x, y )

def lta(loc):
	return location_to_axis(loc)

# Function: location_from_axis( PyInt x, PyInt y )
# Author  : Steve Moret
# Returns : A PyLong representing the location
# Purpose : returns a location represented by the x, and y components x,y
#
def location_from_axis( x, y ):
	# initialize loc to be a LONG integer
	loc = 0L + y
	loc = ( loc << 32 ) + x
	return loc

def lfa(x,y):
	return location_from_axis(x,y)

# Function: set_end_slides( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up all the end slides for the end game
#
def	set_end_slides( attachee, triggerer ):
	pass
	return

# Function: set_join_slides( npc, pc )
# Author  : Tom Decker
# Returns : nada
# Purpose : queues up the end slides if you join the temple
#
def	set_join_slides( attachee, triggerer ):
	pass
	return

# Function: should_heal_hp_on( obj )
# Author  : Tom Decker
# Returns : 1 if character is not at full health, else 0
# Purpose : to heal only characters that need it
#
def should_heal_hp_on( obj ):
	cur = obj.stat_level_get( stat_hp_current )
	max = obj.stat_level_get( stat_hp_max )
	if (not cur == max) and (obj.stat_level_get(stat_hp_current) >= -9):
		return 1
	return 0

# Function: should_heal_disease_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is diseased, else 0
# Purpose : to heal only characters that need it
#
def should_heal_disease_on( obj ):
    # check if obj is diseased
	if (obj.stat_level_get(stat_hp_current) >= -9):
		if obj.d20_query(Q_Critter_Is_Diseased):
			return 1
	return 0

# Function: should_heal_poison_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is poisoned, else 0
# Purpose : to heal only characters that need it
#
def should_heal_poison_on( obj ):
	# check if obj has poison on them
	if (obj.stat_level_get(stat_hp_current) >= -9):
		if obj.d20_query(Q_Critter_Is_Poisoned):
			return 1
	return 0

# Function: should_resurrect_on( obj )
# Author  : Tom Decker
# Returns : 1 if obj is dead, else 0
# Purpose : to heal only characters that need it
#
def should_resurrect_on( obj ):
	if (obj.stat_level_get(stat_hp_current) <= -10):
		return 1
	return 0

def zap( attachee, triggerer ):
	damage_dice = dice_new( '2d4' )
	game.particles( 'sp-Shocking Grasp', triggerer )
	if triggerer.reflex_save_and_damage( OBJ_HANDLE_NULL, 20, D20_Save_Reduction_Half, D20STD_F_NONE, damage_dice, D20DT_ELECTRICITY, D20DAP_UNSPECIFIED, 0 , D20DAP_NORMAL ):
#		saving throw successful
		triggerer.float_mesfile_line( 'mes\\spell.mes', 30001 )
	else:
#		saving throw unsuccessful
		triggerer.float_mesfile_line( 'mes\\spell.mes', 30002 )

	return RUN_DEFAULT

# Added by marc

def should_remove_curse_on (obj):
	# sp_Bestow_Curse_Ability also checks for Actions curse 
	if obj.stat_level_get(stat_hp_current) >= -9:
		if obj.d20_query_has_spell_condition (sp_Bestow_Curse_Ability) or obj.d20_query_has_spell_condition (sp_Bestow_Curse_Rolls):
			return 1
	return 0

def should_remove_blindness_deafness_on (obj):
	if obj.stat_level_get(stat_hp_current) >= -9:
		if obj.d20_query(Q_Critter_Is_Blinded) or obj.d20_query(Q_Critter_Is_Deafened):
			return 1
	return 0

def should_always (obj):
	return 1

#------------------------------------------------------------------------------
# Added by marc, starting 4/26/2016
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Returns a sorted list of all critters in the attachee's group, or enemy group.
#
#   attachee - any creature in the game.
#   group_type - which group to return:  0=friends, 1=enemies, 2=all
#   sort_type - sort the list by:  0=distance, 1=AC, 2=HP
#   exclusions - default bits = '1011001'
#     2**0 - exclude unconscious
#     2**1 - exclude PCs
#     2**2 - exclude NPCs non-summoned: recruits, ai-recruits, animated dead
#     2**3 - exclude NPCs summoned: Monster Summon, Summon Nature's Ally  
#     2**4 - exclude Spiritual Weapons: proto 14370  
#     2**5 - exclude charmed 
#     2**6 - exclude warded: Otiluke's Resilient Sphere
#   return_sort_vals - return a second list with the sort parameter values 
#
# For purposes of this function, every critter falls into one of two groups:
#   1. Party:  All objects in game.party (minus those charmed by Others).
#   2. Others:  All objects Not in game.party (plus those charmed by Others).
#
# Charm concerns: If a critter in Party charms an Other, the charmed critter
# will move to game.party, and all is fine. No need to handle the case.
# However, if a critter in Others charms Party member, that charmed critter
# still exists in game.party, which is handled. (game.leader.group_list() also)
#------------------------------------------------------------------------------
def group_list (attachee, group_type=0, sort_type=0, exclusions='1011001', return_sort_vals=0, print_out=0):

	group = []
	if print_out:
		print " "

	# Party member
	if attachee in game.party:
		if group_type == 0:  # friends
			group = list(game.party)
		elif group_type == 1:  # enemies
			for obj in game.obj_list_vicinity (attachee.location, OLC_CRITTERS):
				if obj not in game.party:
					group.append(obj)
		if group_type == 2:
			group = group + list(game.party)

	# Other NPC, not in party
	if attachee not in game.party:
		if group_type == 0:  # friends
			for obj in game.obj_list_vicinity(attachee.location, OLC_CRITTERS):
				if obj not in game.party:
					group.append(obj)
			for obj in game.party:  # add others charmed by pc
				if obj.d20_query(Q_Critter_Is_Charmed) and obj.type != obj_t_pc:
					group.append(obj)
		elif group_type == 1:  # enemies
			group = list(game.party)
			for obj in game.party:  # remove others charmed by pc
				if obj.d20_query(Q_Critter_Is_Charmed) and obj.type != obj_t_pc:
					group.remove(obj)
		if group_type == 2:
			group = group + list(game.party)

	# Sort the list
	group2 = []
	for obj in group:
		if sort_type == 0:	# distance
			v = max(attachee.distance_to(obj), 0.0)
		elif sort_type == 1:	# AC
			v = obj.stat_level_get(stat_ac)
		elif sort_type == 2:	# HP, current
			v = obj.stat_level_get(stat_hp_current)
		group2.append([v,obj])
	group2.sort()

	x = int(exclusions,2)
	group_objs = []
	group_vals = []

	# Create a list of objects, and also a list of the sort values
	for pair in group2:

		obj = pair[1]
		val = pair[0]

		# Remove critters specified by the exclusions argument.
		if obj.is_unconscious() and x & 2**0:
			continue
		if obj.type == obj_t_pc and x & 2**1:
			continue
		if obj.type != obj_t_pc and obj.d20_query(Q_ExperienceExempt) == 0 and x & 2**2:
			continue
		if obj.type != obj_t_pc and obj.d20_query(Q_ExperienceExempt) == 1 and x & 2**3:
			if obj.name != 14370:
				continue
		if obj.name == 14370 and x & 2**4:
			continue
		if obj.d20_query(Q_Critter_Is_Charmed) and x & 2**5: 
			continue
		if obj.d20_query_has_spell_condition(sp_Otilukes_Resilient_Sphere) and x & 2**6:
			continue

		if print_out:
			print obj, " ", val
		group_objs.append(obj)
		group_vals.append(val)

	if print_out:
		print " "
	if return_sort_vals:
		return group_objs, group_vals

	return group_objs

#------------------------------------------------------------------------------
# Returns a list of all critters within the specified radius of attachee.
# The list is sorted by closest first.
#------------------------------------------------------------------------------
def group_list_radius (attachee, group_type=0, sort_type=0, exclusions='1011001', radius=2.5):
	group_in_radius = []
	for obj in group_list (attachee, group_type, sort_type, exclusions):
		if attachee.distance_to(obj) < radius:
			group_in_radius.append(obj)
	return group_in_radius

#------------------------------------------------------------------------------
# Strategy functions.
#------------------------------------------------------------------------------
def get_strat_num (strategy_name):
	n, strategy_num = 0, -1
	file_in = file('rules\\strategy.tab','r')
	for line in file_in.readlines():
		if strategy_name.lower() == line[0:line.find('\t')].lower():
			strategy_num = n
			break
		n += 1
	file_in.close()
	return strategy_num

def get_strat_name (strategy_num):
	n, strategy_name = 0, ""
	file_in = file('rules\\strategy.tab','r')
	for line in file_in.readlines():
		if strategy_num == n:
			strategy_name = line[0:line.find('\t')].lower()
			break
		n += 1
	file_in.close()
	return strategy_name

def set_strat (attachee, strategy_name):
	n = get_strat_num(strategy_name)
	if n >= 0: 
		attachee.obj_set_int(obj_f_critter_strategy, n)
		float_mes(attachee,219,1)
		float_strat(attachee,n,1)

#------------------------------------------------------------------------------
# Misc. Functions.
#------------------------------------------------------------------------------
def stop_combat (obj, flag=0):
	critters = list(game.obj_list_cone(obj, OLC_CRITTERS, 150, 0, 360)) + [obj]
	for pc in game.party:
		for crit in critters:
			crit.ai_shitlist_remove(pc)  # sets reaction to 50
			crit.reaction_set(pc,50)
			if flag == 1:
				crit.npc_flag_unset(ONF_KOS)  # won't unset if KOS is set in protos.tab
				crit.npc_flag_set(ONF_KOS_OVERRIDE)
		if not crit.critter_flags_get() & OCF_IS_CONCEALED:
			crit.object_flag_unset(OF_CLICK_THROUGH)
			crit.fade_to(255, 1, 20)
	return

def class_levels (obj):
	levels = 0
	for Class in range (stat_level_barbarian, stat_level_wizard + 1):
		levels += obj.stat_level_get(Class)
	return levels

def roll_dice (dice_qty=1, die_size=20, roll_plus=0):
	roll = roll_plus
	for die in range(0,dice_qty):
		roll += game.random_range(1,die_size)
	return roll

def hp_current (npc):
	return npc.stat_level_get(stat_hp_current) - npc.stat_level_get(stat_subdual_damage)

def hp_percent (npc):
	return 100 * (hp_current(npc) * 1.0 / npc.stat_level_get(stat_hp_max))

def is_in_party (object_name):
	for obj in game.party:
		if obj.name == object_name:
			return 1
	return 0

def is_off (npc):
	if npc.object_flags_get() & OF_OFF:
		return 1
	return 0

def party_gender():
	m,f = 0,0
	for friend in game.party:
		if friend.stat_base_get(stat_gender) == gender_female:
			f = 1
		else:
			m = 1
	if m == 1 and f == 1:
		return 2
	elif m == 1:
		return 1
	return 0

def party_skill(skill):
	high = -99
	for pc in game.party:
		if pc.skill_level_get(skill) > high:
			high = pc.skill_level_get(skill)
	return high

def inv_id(attachee):
	return attachee.obj_get_int(obj_f_critter_inventory_source)

def knock_out(npc):
	if hp_current(npc) >= 0:
		dam = hp_current(npc) + 2  
		times = (dam/100)  # maximum number of dice allowed by dice_new() is 127
		for t in range (0, times):  # so for critters with high HPs, break into 100 HP chunks
			npc.damage (OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new("100d1"))
		dam = dam - (times*100)
		# Must heal 1 hp, since doing subdual damage when hp==0 doesn't knock them out. bug? 
		npc.heal (OBJ_HANDLE_NULL, dice_new("1d1"))
		npc.damage (OBJ_HANDLE_NULL, D20DT_SUBDUAL, dice_new(str(dam)+"d1"))


#------------------------------------------------------------------------------
# Debugging functions.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# The message will float if the category for the bit provided in the third
# argument is set in game.global_vars[900]. This provides a way of only
# displaying the float messages currently needed, which prevents an excessive
# flow of messages.
#
# The current categories are:
#    bit 0: general messages (white,0)
#    bit 1: combat (red,1)
#    bit 2: combat spellcasting, including creature abilities (lt.blue,5)
#    bit 3: inventory restock, equipping (green,2)
#    bit 4: transformations, hair, portraits, etc. (yellow,4)
#    bit 5: dungeon spawning (white,0)
#    bit 6: cove zone (white,0)
#------------------------------------------------------------------------------

float_colors = {0:0, 1:1, 2:5, 3:2, 4:4, 5:0, 6:0}

def float_mes (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\marc.mes', line, color)

def float_num (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\nums.mes', line, color)

def float_stat (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\stat.mes', line, color)

def float_stat_enum (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('rules\\stat_enum.mes', line, color)

def float_strat (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\strategy.mes', line, color)

def float_spell (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\spell.mes', line, color)

def float_description (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\description.mes', line, color)

def float_feat (attachee, line, cat=0, color=-1):
	if game.global_vars[900] & 2**cat:
		if color == -1:
			color = float_colors[cat]
		attachee.float_mesfile_line ('mes\\feat.mes', line, color)

def get_mesfile_line (mes_num, mes_file='mes\\marc.mes'):
	File = file(mes_file,'r')
	mes = 'NULL'
	for line in File.readlines():
		line = line.strip()
		if line:
			if line[0] == '{':
				if int(line.split('}')[:-1][0][1:].strip()) == mes_num:
					mes = line.split('}')[:-1][1][1:].strip()
	File.close()
	return mes

def dbug (s, v=-99, file_out='dbug'):
	if game.global_vars[909] == 1:
		if file_out in ("spawn","chests","keys"):
			myfile = file('_dungeon_'+file_out+'.txt','a')
		else:
			myfile = file('_dbug_'+file_out+'.txt','a')
		myfile.write (s)
		if v <> -99:
			if s.strip() == "":
				myfile.write (str(v))
			else:
				myfile.write (" = "+str(v))
		myfile.write ('\n')
		myfile.close()















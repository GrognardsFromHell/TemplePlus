from toee import *

from utilities import *
from batch import *
from itt import *

from math import sqrt, atan2

import _include
from co8Util.PersistentData import *
from Co8 import *

SCRIPT_DAEMON_OUTDOOR_MAPS = [5051, 5095, 5094, 5068, 5002, 5091, 5134, 5069, 5062, 5135]

## Contained in this script


#########################################
# Persistent flags/vars/strs		#
# Uses keys starting with		#
# 'Flaggg', 'Varrr', 'Stringgg' 	#
#########################################

def get_f(flagkey):
	flagkey_stringized = 'Flaggg' + str(flagkey)
	tempp = Co8PersistentData.getData(flagkey_stringized)
	if isNone(tempp):
		return 0
	else:
		return int(tempp) != 0

def set_f(flagkey, new_value = 1):
	flagkey_stringized = 'Flaggg' + str(flagkey)
	Co8PersistentData.setData(flagkey_stringized, new_value)

def get_v(varkey):
	varkey_stringized = 'Varrr' + str(varkey)
	tempp = Co8PersistentData.getData(varkey_stringized)
	if isNone(tempp):
		return 0
	else:
		return int(tempp)

def set_v(varkey, new_value):
	varkey_stringized = 'Varrr' + str(varkey)
	Co8PersistentData.setData(varkey_stringized, new_value)
	return get_v(varkey)

def inc_v(varkey, inc_amount = 1):
	varkey_stringized = 'Varrr' + str(varkey)
	Co8PersistentData.setData(varkey_stringized, get_v(varkey) + inc_amount)
	return get_v(varkey)



def get_s(strkey):
	strkey_stringized = 'Stringgg' + str(strkey)
	tempp = Co8PersistentData.getData(strkey_stringized)
	if isNone(tempp):
		return ''
	else:
		return str(tempp)

def set_s(strkey, new_value):
	new_value_stringized = str(new_value)
	strkey_stringized = 'Stringgg' + str(strkey)
	Co8PersistentData.setData(strkey_stringized, new_value_stringized)


#########################################
# Bitwise NPC internal flags			#
# 1-31									#
# Uses obj_f_npc_pad_i_4			 	#
# obj_f_pad_i_3 is sometimes nonzero    #
# pad_i_4, pad_i_5 tested clean on all  #
# protos								#
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

def npc_unset(attachee,flagno):
	# flagno is assumed to be from 1 to 31
	exponent = flagno - 1
	if exponent > 30 or exponent < 0:
		print 'error!'
	else:
		abc = pow(2,exponent)
	tempp = (attachee.obj_get_int(obj_f_npc_pad_i_4) | abc) - abc
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




################################################################
################################################################
################################################################
################################################################



def san_dying(attachee, triggerer):
	# in case the 'script bearer' dies, pass the curse to someone else
	not_found = 1
	for pc in game.party:
		if pc.stat_level_get( stat_hp_current ) > 0 and not_found == 1 and pc.type == obj_t_pc:
			not_found = 0
			attachee.scripts[12] = 0
			attachee.scripts[38] = 0
			pc.scripts[12] = 648 #san_dying
			pc.scripts[38] = 648 #san_new_map
			pc.scripts[14] = 648 #san_exit_combat - executes when exiting combat mode
			return

def san_exit_combat( attachee, triggerer ):
	dummy = 1
	return
			
def san_new_map( attachee, triggerer ):
	
	game.global_vars[281] = 0 # worldmap "steps" counter - reset to 0 upon arriving
	
	cur_map = attachee.map
	
	if (game.global_vars[57] == 3 or game.global_vars[57] == (3 | (2**10) ) ) and cur_map in SCRIPT_DAEMON_OUTDOOR_MAPS: # Raider Encounter scripting - will queue raider encounter after having visited an outdoor map, so that you encounter them on the way back to the keep
		game.global_vars[57] += 1
		game.encounter_queue.append(3000) # will set flag 277 once it's encountered

	
	try:
		
		file_list = ['001C0024.jpg', '001C0025.jpg', '001C0026.jpg', '001D0024.jpg', '001D0025.jpg', '001D0026.jpg', '001D0027.jpg', '001D0028.jpg', '001D0029.jpg', '001E0024.jpg', '001E0025.jpg', '001E0026.jpg', '001E0027.jpg', '001E0028.jpg', '001E0029.jpg', '001F0024.jpg', '001F0025.jpg', '001F0026.jpg', '001F0027.jpg', '001F0028.jpg', '001F0029.jpg', '001F002A.jpg', '00200025.jpg', '00200026.jpg', '00200027.jpg', '00200028.jpg', '00200029.jpg', '0020002A.jpg', '00210027.jpg', '00210028.jpg', '00210029.jpg', '0021002A.jpg' ]
		orig_dir_day = '//art/ground/KEEP_OUTER_01_Outer_Bailey-day_orig_dupl/'
		orig_dir_night = '//art/ground/KEEP_OUTER_01_Outer_Bailey-night_orig_dupl/'
		
		alt_dir_day = '//art/ground/KEEP_OUTER_01_Outer_Bailey-day_alt/'
		alt_dir_night = '//art/ground/KEEP_OUTER_01_Outer_Bailey-night_alt/'
		
		work_dir_day = '//art/ground/KEEP_OUTER_01_Outer_Bailey-day/'
		work_dir_night = '//art/ground/KEEP_OUTER_01_Outer_Bailey-night/'

		i_file = open('modules\\ToEE\\church_tile_state.mes','r')

		s = i_file.readline()
		param_value = int(s)
		i_file.close()
		
		if param_value == 0: # graphics correspond to intact church

			if game.global_vars[452] & 2**0 != 0: # church should be burnt
				
				alter_tiles(work_dir_day, file_list, alt_dir_day)
				
				alter_tiles(work_dir_night, file_list, alt_dir_night)
				
				i_file = open('modules\\ToEE\\church_tile_state.mes','w')
				i_file.write('1') # marks the file state as 'church burnt'
				i_file.close()
				
			else:
				it_is_ok = 1 # do nothing, the graphics are as they should be
		elif param_value == 1: # graphics correspond to burnt church
			if game.global_vars[452] & 2**0 == 0: # church should be intact
				alter_tiles(work_dir_day, file_list, orig_dir_day)
				alter_tiles(work_dir_night, file_list, orig_dir_night)
				
				i_file = open('modules\\ToEE\\church_tile_state.mes','w')
				i_file.write('0') # marks the file state as 'church burnt'
				i_file.close()
			else:
				it_is_ok = 1 # do nothing, the graphics are as they should be
	except:
		dummy = 1
	
	#######################################
	### Global Event Scheduling System  ###
	#######################################

	## Bethany Encounter - 2 days
	if tpsts('s_bethany', 2*24*60*60) == 1 and get_f('s_bethany_scheduled') == 0:
		set_f('s_bethany_scheduled')
		if game.global_flags[724] == 0 and not (3447 in game.encounter_queue): 
			# ggf724 - have had Bethany Encounter
			game.encounter_queue.append(3447)

	if tpsts('s_zuggtmoy_banishment_initiate', 4*24*60*60) == 1 and get_f('s_zuggtmoy_gone') == 0 and game.global_flags[326] == 1 and attachee.map != 5013 and attachee.map != 5019:
		set_f('s_zuggtmoy_gone')
		import py00262burne_apprentice
		py00262burne_apprentice.return_Zuggtmoy( game.leader, game.leader )

	

	##############################################
	### End of Global Event Scheduling System  ###
	##############################################
	if game.global_vars[449] & (2**0 + 2**1 + 2**2) != 0: # If set preference for speed
		speedup(game.global_vars[449] & (2**0 + 2**1 + 2**2) , game.global_vars[449] & (2**0 + 2**1 + 2**2) )
	if game.global_flags[403] == 1: # Test mode enabled; autokill critters!
		#game.particles( "sp-summon monster I", game.leader)

		#game.timevent_add( autokill, (cur_map, 1), 150 )
		autokill(cur_map, autoloot = 1)
		for pc in game.party:
			pc.identify_all()

	return RUN_DEFAULT


################################################################
################################################################
################################################################
################################################################



def is_follower(name):
	for obj in game.party:
		if (obj.name == name):
			return 1
	return 0




def destroy_weapons(npc, item1, item2, item3):
	if (item1 != 0):
		moshe = npc.item_find(item1)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item2 != 0):
		moshe = npc.item_find(item2)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	if (item3 != 0):
		moshe = npc.item_find(item3)
		if (moshe != OBJ_HANDLE_NULL):
			moshe.destroy()
	return


def float_comment(attachee, line):
	attachee.float_line(line,game.leader)
	return
	
	
def daemon_float_comment(attachee, line):
	if attachee.type == obj_t_pc:
		attachee.scripts[9] = 648
		attachee.float_line(line,game.leader)
		attachee.scripts[9] = 0
	elif type(line) == type(OBJ_HANDLE_NULL) and type(attachee) == type(1):
		daemon_float_comment(line, attachee)
	return

def daemon_float_line(attachee, line):
	daemon_float_comment(attachee, line)
	
def proactivity(npc,line_no):
	npc.turn_towards(game.party[0])
	if (critter_is_unconscious(game.party[0]) != 1 and game.party[0].type == obj_t_pc and game.party[0].d20_query(Q_Prone) == 0 and npc.can_see(game.party[0])):
		game.party[0].begin_dialog(npc,line_no)
	else:
		for pc in game.party:
			npc.turn_towards(pc)
			if (critter_is_unconscious(pc) != 1 and pc.type == obj_t_pc and pc.d20_query(Q_Prone) == 0 and npc.can_see(pc)):
				pc.begin_dialog(npc,line_no)
	return


def tsc( var1, var2 ):
#time stamp compare
#check if event associated with var1 happened before var2
#if they happened in the same second, well... only so much I can do
	if (get_v(var1) == 0):
		return 0
	elif (get_v(var2) == 0):
		return 1
	elif (get_v(var1) < get_v(var2)):
		return 1
	else:
		return 0

def tpsts(time_var, time_elapsed):
# Has the time elapsed since [time stamp] greater than the specified amount?
	if get_v(time_var) == 0:
		return 0
	if game.time.time_game_in_seconds(game.time) > get_v(time_var) + time_elapsed:
		return 1
	return 0

def record_time_stamp(tvar, time_stamp_overwrite = 0):
	if get_v(str(tvar)) == 0 or time_stamp_overwrite == 1:
		set_v(str(tvar), game.time.time_game_in_seconds(game.time) )
	return


def pop_up_box(message_id):
	# generates popup box ala tutorial (without messing with the tutorial entries...)
	a = game.obj_create(11001, game.leader.location)
	a.obj_set_int(obj_f_written_text_start_line,message_id)
	game.written_ui_show(a)
	a.destroy()
	return



def paladin_fall():
	for pc in game.party:
		pc.condition_add('fallen_paladin')

def vlistxyr( xx, yy, name, radius ):
	greg = []
	for npc in game.obj_list_vicinity( lfa(xx,yy), OLC_NPC ):
		npc_x, npc_y = lta(npc.location)
		dist = sqrt((npc_x-xx)*(npc_x-xx) + (npc_y-yy)*(npc_y-yy))
		if (npc.name == name and dist <= radius):
			greg.append(npc)
	return greg


def can_see2(npc,pc):
	# Checks if there's an obstruction in the way (i.e. LOS regardless of facing)
	orot = npc.rotation ## Original rotation
	nx, ny = location_to_axis(npc.location)
	px, py = location_to_axis(pc.location)
	vx = px-nx
	vy = py-ny
	# (vx, vy) is a vector pointing from the PC to the NPC. 
	# Using its angle, we rotate the NPC and THEN check for sight.
	# After that, we return the NPC to its original facing.
	npc.rotation = 3.14159/2 - ( atan2(vy,vx) + 5*3.14159/4 )
	if npc.can_see(pc):
		npc.rotation = orot
		return 1
	npc.rotation = orot
	return 0

def can_see_party(npc):
	for pc in game.party[0].group_list():
		if can_see2(npc, pc) == 1:
			return 1
	return 0

def is_far_from_party(npc, dist = 20):
	# Returns 1 if npc is farther than specified distance from party
	for pc in game.party[0].group_list():
		if npc.distance_to(pc) < dist:
			return 0
	return 1



def is_safe_to_talk_rfv(npc, pc, radius = 20, facing_required = 0, visibility_required = 1):
	# visibility_required - Capability of seeing PC required (i.e. PC is not invisibile / sneaking)
	#	-> use can_see2(npc, pc)
	# facing_required - In addition, the NPC is actually looking at the PC's direction

	if visibility_required == 0:
		if ( pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	elif visibility_required == 1 and facing_required == 1:
		if ( npc.can_see(pc) == 1 and pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	elif visibility_required == 1 and facing_required != 1:
		if ( can_see2(npc, pc) == 1 and pc.type == obj_t_pc and critter_is_unconscious(pc) != 1 and npc.distance_to(pc) <= radius):
			return 1
	return 0


def within_rect_by_corners(obj, ulx, uly, brx, bry):
	# refers to "visual" axes (edges parallel to your screen's edges rather than ToEE's native axes)
	xx, yy = location_to_axis(obj.location)
	if ( (xx - yy) <= (ulx-uly)) and ( (xx - yy) >= (brx-bry) ) and ( (xx + yy) >= (ulx + uly) ) and ( (xx+yy) <= (brx+bry) ):
		return 1
	return 0

	
def encroach(a,b):
	# A primitive way of making distant AI combatants who don't close the distances by themselves move towards the player
	b.turn_towards(a)
	if a.distance_to(b) < 30:
		return -1
	ax,ay = location_to_axis(a.location)
	bx,by = location_to_axis(b.location)
	dx = 0
	dy = 0
	if bx > ax:
		dx = 1
	elif bx < ax:
		dx = -1
	if by > ay:
		dy = 1
	elif by < ay:
		dy = -1
	if (ax-bx)**2 > (ay-by)**2: # if X distance is greater than Y distance, starting trying to encroach on the x axis
		aprobe = game.obj_create( 14631, location_from_axis(ax+dx, ay) ) # probe to see if I'm not going into a wall
		aprobe.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
		if can_see2(aprobe,a):
			aprobe.destroy()
			a.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			return 1
		else:
			aprobe.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			if can_see2(aprobe,a):
				aprobe.destroy()
				a.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				return 1
			else:
				aprobe.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				if can_see2(aprobe,a):
					aprobe.destroy()
					a.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
					return 1
				else:
					aprobe.destroy()
					return 0
	else:
		aprobe = game.obj_create( 14631, location_from_axis(ax+dx, ay) ) # probe to see if I'm not going into a wall
		aprobe.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
		if can_see2(aprobe,a):
			aprobe.destroy()
			a.move(location_from_axis(ax, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			return 1
		else:
			aprobe.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
			if can_see2(aprobe,a):
				aprobe.destroy()
				a.move(location_from_axis(ax+dx, ay+dy) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				return 1
			else:
				aprobe.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
				if can_see2(aprobe,a):
					aprobe.destroy()
					a.move(location_from_axis(ax+dx, ay) , a.obj_get_int(obj_f_offset_x), a.obj_get_int(obj_f_offset_y) )
					return 1
				else:
					aprobe.destroy()
					return 0
	return 0

def buffee( makom , det_range, buff_list, done_list ):
	# finds people that are on a 'to buff' list "buff_list" (name array), around location "makom", at range "det_range", that are not mentioned in "done_list"
	# e.g. in Alrrem's script you can find something like buffee( attachee.location, 15, [14344], [handle_to_other_werewolf] )
	xx0, yy0 = location_to_axis(makom)
	for darling in buff_list:
		for obj in game.obj_list_vicinity( makom, OLC_NPC ):
			xx1, yy1 = location_to_axis( obj.location )
			if obj.name == darling and obj.leader_get() == OBJ_HANDLE_NULL and not (obj in done_list) and ( (xx1-xx0)**2+ (yy1-yy0)**2 ) <= det_range**2:
				return obj
	return OBJ_HANDLE_NULL









def lnk(loc_0 = -1, xx = -1, yy = -1, name_id = -1, stun_name_id = -1):
	# Locate n' Kill!

	if type(stun_name_id) == type(-1):
		stun_name_id = [stun_name_id]
	if type(name_id) == type(-1):
		name_id = [name_id]

	if loc_0 == -1 and xx == -1 and yy == -1:
		loc_0 = game.leader.location
	elif xx != -1 and yy != -1:
		loc_0 = location_from_axis(xx, yy) # Needs location_from_axis from utilities.py
	else:
		loc_0 = game.leader.location

	if name_id == [-1]:
		for obj in game.obj_list_vicinity(loc_0, OLC_NPC):
			if (    obj.reaction_get(game.party[0]) <= 0 or obj.is_friendly(game.party[0]) == 0     )   and     ( obj.leader_get() == OBJ_HANDLE_NULL and obj.object_flags_get() & OF_DONTDRAW == 0):
				if not obj.name in stun_name_id:
					damage_dice = dice_new( '50d50' )
					obj.damage( game.party[0], 0, damage_dice )
					obj.damage( game.party[0], D20DT_FIRE, damage_dice )
					obj.damage( game.party[0], D20DT_COLD, damage_dice )
					obj.damage( game.party[0], D20DT_MAGIC, damage_dice )
				else:
					damage_dice = dice_new( '50d50' )
					obj.damage( OBJ_HANDLE_NULL, D20DT_SUBDUAL, damage_dice )
	else:
		for obj in game.obj_list_vicinity(loc_0, OLC_NPC):
			if obj.name in (name_id+stun_name_id) and ( obj.reaction_get(game.party[0]) <= 0 or obj.is_friendly(game.party[0]) == 0) and (obj.leader_get() == OBJ_HANDLE_NULL and obj.object_flags_get() & OF_DONTDRAW == 0):
				if not (obj.name in stun_name_id):
					damage_dice = dice_new( '50d50' )
					obj.damage( game.party[0], D20DT_BLUDGEONING, damage_dice )
					obj.damage( game.party[0], D20DT_FIRE, damage_dice )
					obj.damage( game.party[0], D20DT_COLD, damage_dice )
					obj.damage( game.party[0], D20DT_MAGIC, damage_dice )
				else:
					damage_dice = dice_new( '50d50' )
					if is_unconscious(obj) == 0:
						obj.damage( OBJ_HANDLE_NULL, D20DT_SUBDUAL, damage_dice )
					for pc in game.party:
						obj.ai_shitlist_remove( pc )


	return

def loot_items( loot_source = OBJ_HANDLE_NULL, pc=-1 , loot_source_name = -1, xx = -1, yy = -1, item_proto_list = [], loot_money_and_jewels_also = 1, autoloot = 1, autoconvert_jewels = 1, item_autoconvert_list = []):
	if get_f('qs_autoloot') != 1:
		return
	if get_f('qs_autoconvert_jewels') != 1:
		autoconvert_jewels = 0
	money_protos = range(7000, 7004) # Note that the range actually extends from 7000 to 7003
	gem_protos = [12010] + range(12034, 12045)
	jewel_protos = range(6180, 6198)
	potion_protos = [8006, 8007]

	tank_armor_0 = []
	barbarian_armor_0 = []
	druid_armor_0 = []
	wizard_items_0 = []

	autosell_list = []
	autosell_list += range(4002, 4106 )
	autosell_list += range(4113, 4120)
	autosell_list += range(4155, 4191)
	autosell_list += range(6001, 6048)
	autosell_list += [6055, 6056] + [6059, 6060]  + range(6062, 6073)
	autosell_list += range(6074, 6082)
	autosell_list += [6093, 6096, 6103, 6120, 6123, 6124]
	autosell_list += range(6142, 6153)
	autosell_list += range(6153, 6159)
	autosell_list += range(6163, 6180)
	autosell_list += range(6202, 6239 )

	autosell_exclude_list = []
	autosell_exclude_list += [4016, 4017, 4025, 4028] # Frag, Scath, Excal, Flam Swo +1
	autosell_exclude_list += [4047, 4057, 4058] # Scimitar +1, Dagger +2, Dager +1
	autosell_exclude_list += [4078, 4079] # Warha +1, +2
	autosell_exclude_list += range(4081, 4087) # Longsword +1 ... +5, Unholy Orc ax+1
	autosell_exclude_list += [4098] # Battleaxe +1
	autosell_exclude_list += [4161] # Shortsword +2
	autosell_exclude_list += [5802] # Figurine name IDs - as per protos.tab
	autosell_exclude_list += [6015, 6017, 6031, 6039, 6058, 6073, 6214, 6215, 6219]
	autosell_exclude_list += [6239, 12602]
	autosell_exclude_list += [8006, 8007, 8008, 8101] # Potions of Cure mod, serious & Haste
	# 6015 - eye of flame cloak
	# 6017 - gnome ring
	# 6031 - eyeglasses
	# 6039 - Full Plate
	# 6048 - Prince Thrommel's Plate
	# 6058 - Cloak of Elvenkind
	# 6073 - Wooden Elvish Shield
	# 6214, 6215 - Green & Purple (resp.) Elven chain
	# 6219 - Senshock robes
	# 6239 - Darley's Necklace
	# 12602 - Hill Giant's Head
	for qqq in autosell_exclude_list:
		if qqq in autosell_list:
			autosell_list.remove(qqq)

	if loot_money_and_jewels_also == 1:
		if type(item_proto_list) == type([]):
			item_proto_list = item_proto_list + money_protos + gem_protos + jewel_protos + potion_protos
		else:
			item_proto_list = [item_proto_list] + money_protos + gem_protos + jewel_protos + potion_protos
	elif type(item_proto_list) == type(1):
		item_proto_list = [item_proto_list]

	# pc - Who will take the loot?
	if pc == -1:
		pc = game.leader
	# loc_0 - Where will the loot be sought?
	if xx == -1 or yy == -1:
		loc_0 = pc.location
	else:
		loc_0 = location_from_axis(xx, yy)

	if loot_source != OBJ_HANDLE_NULL:
		for pp in (item_proto_list + item_autoconvert_list):
			if type(pp) == type(1):
				if pp in item_autoconvert_list:
					pp_1 = loot_source.item_find_by_proto(pp)
					if pp_1 != OBJ_HANDLE_NULL:
						if pp_1.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
							autosell(pp_1)
				elif pc.item_get( loot_source.item_find_by_proto(pp) ) == 0:
					for obj in game.party:
						if obj.item_get( loot_source.item_find_by_proto(pp) ) == 1:
							break
	else:
		#ff = open('modules\\ToEE\\autolootfeedback.txt','a')
		#ff_s = ''
		if loot_source_name != -1:
			if type(loot_source_name) == type(1):
				loot_source_name = [loot_source_name]
		else:
			loot_source_name = [-1]
		for robee in game.obj_list_vicinity(loc_0, OLC_NPC | OLC_CONTAINER | OLC_ARMOR | OLC_WEAPON | OLC_GENERIC):
			# ff_s = ff_s + str(robee) + ',      name = ' + str(robee.name) + '\n'
			# ff_s = ff_s + str(loot_source_name) + '\n'
			if not robee in game.party[0].group_list() and (robee.name in loot_source_name or loot_source_name == [-1]):
				# ff_s = ff_s + 'NPC / Container Ok to loot!' + '\n'
				if (robee.type == obj_t_weapon) or (robee.type == obj_t_armor) or (robee.type == obj_t_generic):
					if robee.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
						if robee.name in autosell_list + item_autoconvert_list:
							autosell_item(robee)
						elif robee.name in autosell_exclude_list:
							if pc.item_get(robee) == 0:
								for obj in game.party:
									if obj.item_get(robee) == 1:
										break
				if robee.type == obj_t_npc:
					for qq in range(0, 16):
						qq_item_worn = robee.item_worn_at(qq)
						if qq_item_worn != OBJ_HANDLE_NULL and qq_item_worn.item_flags_get() & (OIF_NO_DISPLAY + OIF_NO_LOOT) == 0:
							if qq_item_worn.name in (autosell_list + item_autoconvert_list):
								autosell_item(qq_item_worn)
				for item_proto in (item_proto_list + item_autoconvert_list):
					# ff_s = ff_s + str(item_proto) + '    ' + str(item_proto_list)  + '\n'
					item_sought = robee.item_find_by_proto(item_proto)
					if  item_sought != OBJ_HANDLE_NULL and item_sought.item_flags_get() & OIF_NO_DISPLAY == 0:
						# ff_s = ff_s + 'found item!\n'
						if (  (item_proto in ( gem_protos + jewel_protos ) ) and autoconvert_jewels == 1) or (item_proto in item_autoconvert_list):
							autosell_item(item_sought, item_proto, pc)
						elif pc.item_get(item_sought) == 0:
							for obj in game.party:
								if obj.item_get(item_sought) == 1:
									break
		#ff.write(str(ff_s))
		#ff.close()
	return

def sell_modifier():
	highest_appraise = -999
	for obj in game.party:
		if obj.skill_level_get(skill_appraise) > highest_appraise:
			highest_appraise = obj.skill_level_get(skill_appraise)
	for pc in game.party:
		if pc.stat_level_get(stat_level_wizard) > 1:
			highest_appraise = highest_appraise + 2 # Heroism / Fox's Cunning bonus
			break
	for pc in game.party:
		if pc.stat_level_get(stat_level_bard) > 1:
			highest_appraise = highest_appraise + 2 # Inspire Competence bonus
			break
	if highest_appraise > 19:
		return 0.97
	elif highest_appraise < -13:
		return 0
	else:
		return 0.4 + float(highest_appraise)*0.03

def appraise_tool( obj ):
	# Returns what you'd get for selling it
	aa = sell_modifier()
	return int( aa * obj.obj_get_int(obj_f_item_worth) )

def s_roundoff( app_sum ):
	if app_sum <= 1000:
		return app_sum
	if app_sum > 1000 and app_sum <= 10000:
		return 10 * int( (int(app_sum) / 10 ) )
	if app_sum > 10000 and app_sum <= 100000:
		return 100 * int( (int(app_sum) / 100 ) )
	if app_sum > 100000 and app_sum <= 1000000:
		return 1000 * int( (int(app_sum) / 1000 ) )

def autosell_item(item_sought = OBJ_HANDLE_NULL, item_proto = -1, pc = -1, item_quantity = 1, display_float = 1):
	
	if item_sought == OBJ_HANDLE_NULL:
		return
	if pc == -1:
		pc = game.leader
	if item_proto == -1:
		item_proto = item_sought.name

	autoconvert_copper =  appraise_tool(item_sought) * item_sought.obj_get_int(obj_f_item_quantity)
	pc.money_adj( autoconvert_copper )
	item_sought.object_flag_set(OF_OFF)
	item_sought.item_flag_set( OIF_NO_DISPLAY )
	item_sought.item_flag_set( OIF_NO_LOOT )

	if display_float == 1 and autoconvert_copper > 5000 or display_float == 2:
		pc.float_mesfile_line( 'mes\\script_activated.mes', 10000, 2 )
		pc.float_mesfile_line( 'mes\\description.mes', item_proto, 2 )
		pc.float_mesfile_line( 'mes\\transaction_sum.mes', ( s_roundoff(autoconvert_copper/100) ), 2 )

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


def cnk(proto_id, do_not_destroy = 0, how_many = 1, timer = 0):
	# Create n' Kill
	# Meant to simulate actually killing the critter
	#if timer == 0:
	for pp in range(0, how_many):
		a = game.obj_create(proto_id, game.leader.location)
		damage_dice = dice_new( '50d50' )
		a.damage( game.party[0], 0, damage_dice )
		if do_not_destroy != 1:
			a.destroy()
	#else:
	#	for pp in range(0, how_many):
	#		game.timevent_add( cnk, (proto_id, do_not_destroy, 1, 0), (pp+1)*20 )
	return




################
################
### AUTOKILL ###
################
################







def autokill(cur_map, autoloot = 1, is_timed_autokill = 0):

	#if (cur_map in range(5002, 5078) ): #random encounter maps
	#	## Skole Goons
	#	flash_signal(0)
	#	if get_f('qs_autokill_nulb'):
	#		if get_v('qs_skole_goon_time') == 0:
	#			set_v('qs_skole_goon_time', 500)
	#			game.timevent_add( autokill, (cur_map), 100 )
	#			flash_signal(1)
	#		if get_v('qs_skole_goon_time') == 500:
	#			flash_signal(2)
	#			lnk(name_id = [14315])
	#			#14315 - Skole Goons
	#			loot_items(loot_source_name = [14315]) # Skole goons
		#if get_f('qs_is_repeatable_encounter'):
		#	lnk()
		#	loot_items()

################
###  HOMMLET   #
################

	if (cur_map == 5001): # Hommlet Exterior
		if get_v('qs_emridy_time') == 1500:
			game.quests[100].state = qs_completed
			bro_smith = OBJ_HANDLE_NULL
			for obj in game.obj_list_vicinity(location_from_axis(571, 434), OLC_NPC):
				if obj.name == 20005:
					bro_smith = obj
			if bro_smith != OBJ_HANDLE_NULL:
				party_transfer_to(bro_smith, 12602)
				game.global_flags[979] = 1
			set_v('qs_emridy_time', 2000)


		if get_f('qs_arena_of_heroes_enable'):
			if get_f('qs_lareth_dead'):
				game.global_vars[974] = 2 # Simulate having talked about chest
				game.global_vars[705] = 2 # Simulate having handled chest
				if get_f('qs_book_of_heroes_given') == 0:
					giv(game.leader, 11050, 1) # Book of Heroes
					giv(game.leader, 12589, 1) # Horn of Fog
					set_f('qs_book_of_heroes_given')
				game.global_vars[702] = 1 # Make sure Kent doesn't pester
			if game.global_vars[994] == 0:
				game.global_vars[994] = 1 # Skip Master of the Arena chatter



	if (cur_map == 5046): # Welcome Wench Upstairs
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Barbarian Elf
				lnk(xx=482, yy=476, name_id = 8717)
				loot_items(loot_source_name = 8717, item_autoconvert_list = [6396, 6045, 6046, 4204])
				game.global_vars[961] = 4


##################
###  OAK WOODS   #
##################

	if (cur_map == 5094): # Northern Oak Woods
		if get_f('qs_autokill_oak_woods') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, autoloot, 1 ), 250)
			lnk(xx=588, yy=512, name_id = [14107, 14123, 14124, 14125, 14126, 14127, 14128, 14129, 14136]) # Ghouls / Skeletons / Zombies (damn you Ted and your randomization!)
			#loot_items(xx=475, yy=505, item_proto_list = [6270], loot_source_name = 14057, autoloot = autoloot) # Jay's Ring

			lnk(xx=575, yy=460, name_id = 14050) # Wolf
			loot_items(xx=575, yy=460, item_proto_list = [12800], loot_source_name = 14050, autoloot = autoloot) # Wolf Pelt
			
			lnk(xx=550, yy=420, name_id = [14052, 14053]) # Brown Bear
			
			lnk(xx=488, yy=387, name_id = 14388) # Snake
			
			lnk(xx=410, yy=370, name_id = [14049, 14051]) # Savage Dogs / Jackals
			
			lnk(xx=401, yy=447, name_id = 14345) # Corpser
			
			lnk(xx=533, yy=611, name_id = [14684, 14686, 14687, 14688, 14690]) # Small Undead
			
			lnk(xx=427, yy=532, name_id = 14237) # Harpy Baby
			
			if game.quests[19].state >= qs_mentioned:
				lnk(xx=499, yy=487, name_id = 14237) # Harpy Babies
				lnk(xx=499, yy=475, name_id = [14243, 14237]) # Harpy + Harpy babies
			#loot_items(xx=475, yy=460, autoloot = autoloot)

			#if get_v('qs_moathouse_ambush_time') == 0 and get_f('qs_lareth_dead') == 1:
			#	game.timevent_add( autokill, (cur_map), 500 )
			#	set_v('qs_moathouse_ambush_time', 500)
			#elif get_v('qs_moathouse_ambush_time') == 500:
			#	lnk(xx = 478, yy = 460, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
			#	lnk(xx = 430, yy = 444, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
			#	loot_items(xx=478, yy=460)
			#	loot_items(xx=430, yy=444)
			#	set_v('qs_moathouse_ambush_time', 1000)

		#if get_f('qs_autokill_temple') == 1:
		#	lnk(xx=503, yy=506, name_id = [14507, 14522] ) # Boars
		#	lnk(xx=429, yy=437, name_id = [14052, 14053] ) # Bears
		#	lnk(xx=478, yy=448, name_id = [14600, 14674, 14615, 14603, 14602, 14601] ) # Undead
		#	lnk(xx=468, yy=470, name_id = [14674, 14615, 14603, 14602, 14601] ) # Undead





	if (cur_map == 5095): # Lizard Swamp
		if get_f('qs_autokill_lizard_swamp') == 1:
			game.areas[4] = 1 # Lord Axer's Thorp
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, autoloot, 1 ), 250)
			lnk(xx=461, yy=521, name_id = [14688, 14094, 14386, 14123, 14124, 14126, 14127]) # Lizard Skellies / Crayfish / Zombies / Vipers
			#loot_items(xx=475, yy=505, item_proto_list = [6270], loot_source_name = 14057, autoloot = autoloot) # Jay's Ring

			lnk(xx=465, yy=556, name_id = [14385, 14345, 14090 ]) # Small Vipers / Corpsers / Lizards  (Pool 3)
			
			lnk(xx=486, yy=515, name_id = [14345, 14375, 14057]) # Corpsers / Water Snakes / Frogs (Pool 2)
			
			lnk(xx=510, yy=515, name_id = [14093, 14090, 14057]) # Yabbies / Lizards / Frogs (Pool 1)
			
			lnk(xx=484, yy=450, name_id = [14084, 14085, 14086, 14090]) # Lizardmen group
			
			loot_items(xx=484, yy=450, item_proto_list = [8014], autoloot = autoloot, item_autoconvert_list=[4123])
			
			lnk(xx=468, yy=460, name_id = [14084, 14085, 14086, 14090]) # Lizardmen group 2
			
			loot_items(xx=468, yy=460, item_proto_list = [8014], autoloot = autoloot, item_autoconvert_list=[4123])
			
			lnk(xx=455, yy=476, name_id = [14084, 14085, 14086, 14090]) # Lizardmen group 3
			
			loot_items(xx=455, yy=476, item_proto_list = [8014], autoloot = autoloot, item_autoconvert_list=[4123])
			

	if (cur_map == 5002): # Spiders Woods (Southern Pine Woods)
		if get_f('qs_autokill_spider_woods') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, autoloot, 1 ), 250)
			lnk(xx = 500, yy = 440, name_id = [14387, 14388, 14390]) # Nonspider spawn 1 -  Large Viper / Constrictor Snake / Dire bat
			
			lnk(xx = 525, yy = 450, name_id = [14047, 14397, 14695]) # Spider Spawn 1 - Small Black Widow / Fiendish Small Monstrous Spider
			
			lnk(xx = 559, yy = 444, name_id = [14047, 14399, 14417]) # Spider Spawn 2 - Large Spider / Black Widow / Fiendish Large Monstrous Spider
			
			lnk(xx = 584, yy = 447, name_id = [14520, 14406, 14052, 14053]) # Nonspider spawn 2 -  Fiendish Boar / Fiendish Wolf / Bears
			
			lnk(xx = 390, yy = 415, name_id = [14417, 14397, 14695]) # Spider Spawn 3 - Small Black Widow / Fiendish Small Monstrous Spider
			
			lnk(xx = 482, yy = 580, name_id = [14397, 14695]) # Spider Spawn 4 - Small Black Widow / Fiendish Small Monstrous Spider
			
			lnk(xx = 442, yy = 370, name_id = [14050, 14051]) # Nonspider spawn 3 -  Jackals / Wolves
			
			lnk(xx = 482, yy = 580, name_id = [14398, 14417, 14047]) # Spider Spawn 5 - Large Spider / Black Widow / Fiendish Medium Monstrous Spider
			
			lnk(xx = 577, yy = 506, name_id = [14397, 14417, 14047]) # Spider Silk Spiders
			
			for obj in game.obj_list_vicinity( location_from_axis(577, 507), OLC_GENERIC):
				if obj.name == 12799: # Spider Silk
					game.leader.item_get( obj )
			#loot_items(xx=577, yy=506, autoloot = autoloot, item_proto_list = [12799])
			
			lnk(xx = 552, yy = 527, name_id = [14397, 14417, 14047]) # More Spiders
			
			lnk(xx = 455, yy = 436, name_id = [14397, 14417, 14047]) # More Spiders
			
			if speakers_of_tongue('sylvan') != [OBJ_HANDLE_NULL] and game.quests[18].state < qs_accepted:
				game.quests[18].state = qs_accepted
			

		if get_f('qs_autokill_temple') == 1 and game.global_vars[972] == 2:
			if get_v('qs_moathouse_respawn__upper_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_respawn__upper_time', 500)
			if get_v('qs_moathouse_respawn__upper_time') == 500:
				lnk(xx=476, yy=493, name_id = [14138, 14344, 14391] ) # Lycanthropes
				lnk(xx = 502, yy = 476, name_id = [14295, 14142]) # Basilisk & Ochre Jelly


	if (cur_map == 5114): # Moathouse Dungeon
		if get_f('qs_autokill_moathouse') == 1:

			lnk(xx = 416, yy = 439, name_id = 14065) # Lubash
			loot_items(xx=416, yy=439, item_proto_list = [6058], loot_source_name = 14065 , autoloot = autoloot)
			game.global_flags[55] = 1 # Freed Gnomes
			game.global_flags[991] = 1 # Flag For Verbobonc Gnomes

			lnk(xx = 429, yy = 413, name_id = [14123, 14124, 14092, 14126, 14091]) # Zombies, Green Slime
			lnk(xx = 448, yy = 417, name_id = [14123, 14124, 14092, 14126]) # Zombies
			loot_items(xx=448, yy=417, item_proto_list = 12105, loot_source_name = -1 , autoloot = autoloot)



			lnk(xx = 450, yy = 519, name_id = range(14170, 14174) + range(14213, 14217) ) # Bugbears
			lnk(xx = 430, yy = 524, name_id = range(14170, 14174) + range(14213, 14217) ) # Bugbears
			loot_items(xx=450, yy=519 , autoloot = autoloot)
			loot_items(xx=430, yy=524 , autoloot = autoloot)

			if len(game.party) < 4 and get_v('AK5005_Stage') < 1:
				set_v('AK5005_Stage', get_v('AK5005_Stage') + 1)
				return
				
			# Gnolls and below
			lnk(xx = 484, yy = 497, name_id = [14066, 14067, 14078, 14079, 14080]) # Gnolls
			lnk(xx = 484, yy = 473, name_id = [14066, 14067, 14078, 14079, 14080]) # Gnolls
			loot_items(xx=484, yy=497 , autoloot = autoloot)
			loot_items(xx=484, yy=473 , autoloot = autoloot)

			lnk(xx = 543, yy = 502, name_id = 14094) # Giant Crayfish

			lnk(xx = 510, yy = 447, name_id = [14128, 14129, 14095]) # Ghouls

			if len(game.party) < 4 and get_v('AK5005_Stage') < 2   or  (  len(game.party) < 8 and get_v('AK5005_Stage') < 1    ):
				set_v('AK5005_Stage', get_v('AK5005_Stage') + 1)
				return

			lnk(xx = 515, yy = 547, name_id = [14074, 14075]) # Front Guardsmen
			loot_items(xx=515, yy=547 , autoloot = autoloot)

			lnk(xx = 485, yy = 536, name_id = [14074, 14075, 14076, 14077]) # Back Guardsmen
			loot_items(xx=485, yy=536 , loot_source_name = [14074, 14075, 14076, 14077], autoloot = autoloot) # Back guardsmen

			from py00060lareth import create_spiders
			if get_f('qs_lareth_spiders_spawned') == 0:
				create_spiders(game.leader, game.leader)
				set_f('qs_lareth_spiders_spawned', 1)
			lnk(xx = 480, yy = 540, name_id = [8002, 14397, 14398, 14620]) # Lareth & Spiders
			set_f('qs_lareth_dead')
			lnk(xx = 530, yy = 550, name_id = [14417]) # More Spiders
			loot_items(xx=480, yy=540 , item_proto_list = ([4120, 6097, 6098, 6099, 6100, 11003] + range(9001, 9688) ) , loot_source_name = [8002, 1045], autoloot = autoloot) # Lareth & Lareth's Dresser
			loot_items(xx=480, yy=540, item_autoconvert_list = [4194])
### RESPAWN
		if get_f('qs_autokill_temple') == 1 and game.global_vars[972] == 2:
			if get_v('qs_moathouse_respawn_dungeon_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_respawn_dungeon_time', 500)

			if get_v('qs_moathouse_respawn__upper_time') == 500:
				lnk(xx = 416, yy = 439, name_id = 14141) # Crystal Oozes

				# Bodaks, Shadows and Groaning Spirit
				lnk(xx = 436, yy = 521, name_id = [14328, 14289, 14280]) 

				# Skeleton Gnolls
				lnk(xx = 486, yy = 480, name_id = [14616, 14081, 14082, 14083]) 
				lnk(xx = 486, yy = 495, name_id = [14616, 14081, 14082, 14083]) # Skeleton Gnolls

				# Witch
				lnk(xx = 486, yy = 540, name_id = [14603, 14674, 14601, 14130, 14137, 14328, 14125, 14110, 14680]) 
				loot_items(xx = 486, yy = 540, item_proto_list = [11098, 6273, 4057,6263, 4498], item_autoconvert_list = [4226, 6333, 5099])


	if (cur_map == 5091): # Cave Exit
		if get_f('qs_autokill_moathouse') == 1:
			if get_v('qs_moathouse_ambush_time') == 0 and get_f('qs_lareth_dead') == 1:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_moathouse_ambush_time', 500)
			elif get_v('qs_moathouse_ambush_time') == 500:
				lnk(xx = 500, yy = 490, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				lnk(xx = 470, yy = 485, name_id = [14078, 14079, 14080, 14313, 14314, 14642, 8010, 8004, 8005]) # Ambush
				loot_items(xx=500, yy=490)
				loot_items(xx=470, yy=490)
				set_v('qs_moathouse_ambush_time', 1000)




	if (cur_map == 5069): # Emridy Meadows
		if get_f('qs_autokill_moathouse') == 1:
			if get_v('qs_emridy_time') == 0:
				game.timevent_add( autokill, (cur_map), 500 )
				set_v('qs_emridy_time', 500)
			elif get_v('qs_emridy_time') == 500:
				set_v('qs_emridy_time', 1000)
				game.timevent_add( autokill, (cur_map), 500 )

				lnk(xx = 467, yy = 383, name_id = [14603, 14600]) # NW Skeletons
				loot_items(xx=467, yy=380)

				lnk(xx = 507, yy = 443, name_id = [14603, 14600]) # W Skeletons
				lnk(xx = 515, yy = 421, name_id = [14603, 14600]) # W Skeletons
				loot_items(xx=507, yy=443)
				loot_items(xx=515, yy=421)

				lnk(xx = 484, yy = 487, name_id = [14603, 14600, 14616, 14615]) # Rainbow Rock 1
				lnk(xx = 471, yy = 500, name_id = [14603, 14600, 14616, 14615]) # Rainbow Rock 1
				loot_items(xx=484, yy=487)

				loot_items(xx=484, yy=487, loot_source_name = [1031], item_proto_list = [12024])



				if get_f('qs_rainbow_spawned') == 0:
					set_f('qs_rainbow_spawned', 1)
					#py00265rainbow_rock.san_use(game.leader, game.leader)
					#san_use(game.leader, game.leader)
					#game.particles( "sp-summon monster I", game.leader)
					for qq in game.obj_list_vicinity( location_from_axis(484, 487), OLC_CONTAINER ):
						if qq.name == 1031:
							qq.object_script_execute( qq, 1 )
				lnk(xx = 484, yy = 487, name_id = [14602, 14601]) # Rainbow Rock 2
				loot_items(xx=484, yy=487)


				#game.timevent_add( autokill, (cur_map), 1500 )

				lnk(xx = 532, yy = 540, name_id = [14603, 14600]) # SE Skeletons
				loot_items(xx=540, yy=540)



				lnk(xx = 582, yy = 514, name_id = [14221, 14053]) # Hill Giant
			elif get_v('qs_emridy_time') == 1000:
				set_v('qs_emridy_time', 1500)
				loot_items(xx=582, yy=514)
				loot_items(xx=582, yy=514, item_proto_list = [12602])
				if game.leader.item_find_by_proto(12602) == OBJ_HANDLE_NULL:
					create_item_in_inventory(12602, game.leader)

##################
###  NULB	 #
##################


	if (cur_map == 5051): # Nulb Outdoors

		if get_f('qs_autokill_temple') == 1:
			game.global_vars[972] = 2 # Simulate Convo with Kent

		if get_f('qs_autokill_nulb') == 1:
			# Spawn assassin
			game.global_flags[277] = 1 # Have met assassin
			game.global_flags[292] = 1
			if get_f('qs_assassin_spawned') == 0:
				a = game.obj_create(14303, game.leader.location)
				lnk(name_id = 14303)
				loot_items(loot_source_name = 14303, item_proto_list = [6315, 6199, 4701, 4500, 8007, 11002], item_autoconvert_list = [6046])
				set_f('qs_assassin_spawned')

			game.global_flags[356] = 1 # Met Mickey
			game.global_flags[357] = 1 # Mickey confessed to taking Orb
			game.global_flags[321] = 1 # Met Mona
			record_time_stamp('s_skole_goons')

			game.quests[41].state = qs_completed # Preston's Tooth Ache
			game.global_flags[94] = 1 # Nulb House is yours	
			game.global_flags[315] = 1 # Purchased Serena's Freedom	
			game.quests[60].state = qs_completed # Mona's Orb
			game.quests[63].state = qs_completed # Bribery for justice


			if get_f('qs_killed_gar') == 1:
				game.quests[35].state = qs_completed # Grud's story
				game.leader.reputation_add( 25 )


	if (cur_map == 5068): # Imeryd's Run
		if get_f('qs_autokill_nulb') == 1:
			lnk(xx = 485, yy = 455, name_id = ([14279] + range(14084, 14088))  ) # Hag & Lizards
			#lnk(xx = 468, yy = 467, name_id = ([14279] + range(14084, 14088))  ) # Hag & Lizards
			loot_items(xx=485, yy = 455)


			lnk(xx = 460, yy = 480, name_id = [14329]) # Gar
			loot_items(xx=460, yy=480, item_proto_list = [12005]) # Gar Corpse + Lamia Figurine
			loot_items(xx=460, yy=500, item_proto_list = [12005]) # Lamia Figurine - bulletproof
			set_f('qs_killed_gar')


			lnk(name_id = [14445, 14057]) # Kingfrog, Giant Frog
			loot_items(xx=476, yy = 497, item_proto_list = [4082, 6199, 6082, 4191, 6215, 5006])

	if (cur_map == 5052): # Boatmen's Tavern
		if get_f('qs_autokill_nulb') == 1:
			if game.global_flags[281] == 1: # Have had Skole Goons Encounter
				lnk(name_id = [14315, 14134]) # Skole + Goon
				loot_items(loot_source_name = [14315, 14134], item_proto_list = [6051, 4121])
				for obj_1 in game.obj_list_vicinity(game.leader.location, OLC_NPC):
					for pc_1 in game.party[0].group_list():
						obj_1.ai_shitlist_remove( pc_1 )
						obj_1.reaction_set( pc_1, 50 )

	if (cur_map == 5059): # Snakepit Brothel
		if get_f('qs_autokill_nulb') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				lnk(xx = 508, yy= 485, name_id = 8718)
				loot_items(xx = 508, yy = 485, loot_source_name = 8718, item_autoconvert_list = [4443, 6040, 6229])
				game.global_vars[961] = 6


	if (cur_map == 5054): # Waterside Hostel
		if get_f('qs_autokill_nulb') == 1:
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
			# Thieving Dala
				game.quests[37].state = qs_completed
				lnk(xx = 480, yy= 501, name_id = [14147, 14146, 14145, 8018, 14074], stun_name_id = [14372, 14373])
				loot_items(xx=480, yy= 501, loot_source_name = [14147, 14146, 14145, 8018, 14074])
				for obj_1 in game.obj_list_vicinity(location_from_axis(480, 501), OLC_NPC):
					for pc_1 in game.party[0].group_list():
						obj_1.ai_shitlist_remove( pc_1 )
						obj_1.reaction_set( pc_1, 50 )

				

##########################
###  HICKORY BRANCH	 #
##########################

	if (cur_map == 5062): # Hickory Branch Exterior
		if get_f('qs_autokill_nulb'):
			# First party, near Noblig
			lnk(xx = 433, yy = 538, name_id = [14467, 14469, 14470, 14468, 14185]) 
			loot_items(xx=433, yy = 538, item_autoconvert_list = [4201, 4209, 4116, 6321]) # Shortbow, Spiked Chain, Short Spear, Marauder Armor

			# NW of Noblig
			lnk(xx = 421, yy = 492, name_id = [14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=421, yy = 492, item_autoconvert_list = [4201, 4209, 4116])

			# Wolf Trainer Group
			lnk(xx = 366, yy = 472, name_id = [14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=366, yy = 472, item_autoconvert_list = [4201, 4209, 4116])

			# Ogre Shaman Group
			lnk(xx = 449, yy = 455, name_id = [14249, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=449, yy = 455, item_autoconvert_list = [4201, 4209, 4116])

			# Orc Shaman Group
			lnk(xx = 494, yy = 436, name_id = [14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=494, yy = 436, item_autoconvert_list = [4201, 4209, 4116])

			# Cave Entrance Group
			lnk(xx = 527, yy = 380, name_id = [14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=527, yy = 380, item_autoconvert_list = [4201, 4209, 4116])
			
			# Dire Bear
			lnk(xx = 548, yy = 430, name_id = [14506])

			# Cliff archers
			lnk(xx = 502, yy = 479, name_id = [14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=502, yy = 479, item_autoconvert_list = [4201, 4209, 4116])
			
			# Giant Snakes
			lnk(xx = 547, yy = 500, name_id = [14449])
			loot_items(xx=547, yy = 500, item_autoconvert_list = [4201, 4209, 4116])

			# Owlbear
			lnk(xx = 607, yy = 463, name_id = [14046])


			# Dokolb area
			lnk(xx = 450, yy = 519, name_id = [14640, 14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=450, yy = 519, item_autoconvert_list = [4201, 4209, 4116])


			# South of Dokolb Area
			lnk(xx = 469, yy = 548, name_id = [14188, 14465, 14249, 14743, 14747, 14749, 14745, 14746, 14482, 14093, 14067, 14466, 14352, 14467, 14469, 14470, 14468, 14185, 14050, 14391])
			loot_items(xx=469, yy = 548, item_autoconvert_list = [4201, 4209, 4116])


	if (cur_map == 5065): # Hickory Branch Cave
		if get_f('qs_autokill_nulb'):
			if get_v('qs_hickory_cave_timer') == 0:
				set_v('qs_hickory_cave_timer', 500) 
				game.timevent_add(autokill, (cur_map), 500)
			if get_v('qs_hickory_cave_timer') == 500:
				lnk()
				loot_items(item_proto_list = [4086, 6106, 10023], item_autoconvert_list = [6143, 4110, 4241, 4242, 4243, 6066, 4201, 4209, 4116])
				loot_items(xx = 490, yy = 453, item_proto_list = [4078, 6252, 6339, 6091], item_autoconvert_list = [6304, 4240, 6161, 6160, 4087, 4204])


	if (cur_map == 5191): # Minotaur Lair
		if get_f('qs_autokill_nulb'):
			lnk(xx = 492, yy = 486)
			loot_items(492, 490, item_proto_list = [4238, 6486, 6487])


##########################
###  ARENA OF HEROES	 #
##########################

	if (cur_map == 5119): # AoH
		if get_f('qs_autokill_temple'):
			#game.global_vars[994] = 3
			dummy = 1


##########################
###  MOATHOUSE RESPAWN	 #
##########################

	if (cur_map == 5120): # Forest Drow
		#flash_signal(0)
		if get_f('qs_autokill_temple'):
			lnk(xx = 484, yy = 481, name_id = [14677, 14733, 14725, 14724, 14726])
			loot_items(xx = 484, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076])




##################################
###  TEMPLE OF ELEMENTAL EVIL	 #
##################################


	if (cur_map == 5111): # Tower Sentinel
		if get_f('qs_autokill_temple'):
			lnk(xx = 480, yy = 490, name_id = 14157)
			loot_items(xx = 480, yy = 490)

	if (cur_map == 5133): # Brigand Tower
		if get_f('qs_autokill_temple'):
			lnk(xx = 477, yy = 490, name_id = [14314, 14313, 14312, 14310, 14424, 14311, 14425])
			lnk(xx = 490, yy = 480, name_id = [14314, 14313, 14312, 14310, 14424, 14311, 14425])

			loot_items(item_proto_list = [10005, 6051], item_autoconvert_list = [4081, 6398, 4067])
			loot_items(xx = 490, yy = 480, item_proto_list = [10005, 6051], item_autoconvert_list = [4081, 6398, 4067, 4070, 4117, 5011])


	if (cur_map == 5005): # Temple Level 1 - Earth Floor
		if get_f('qs_autokill_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				
				#Stirges
				lnk(xx = 415, yy = 490, name_id = [14182])

				# Harpies & Ghouls
				lnk(xx = 418, yy = 574, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 401, yy = 554, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 401, yy = 554, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 421, yy = 544, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				lnk(xx = 413, yy = 522, name_id = [14095, 14129, 14243, 14128, 14136, 14135])
				loot_items(xx = 401, yy = 554)


				# Gel Cube + Grey Ooze
				lnk(xx = 407, yy = 594, name_id = [14095, 14129, 14139, 14140])
				loot_items(xx = 407, yy = 600, loot_source_name = [14448, 1049], item_autoconvert_list = [4121, 4118, 4113, 4116, 5005, 5098])


				# Corridor Ghouls
				lnk(xx = 461, yy = 600, name_id = [14095, 14129])
				
				# Corridor Gnolls
				lnk(xx = 563, yy = 600, name_id = [14078, 14079, 14080])
				loot_items(xx = 563, yy = 600, loot_source_name = [14078, 14079, 14080, 1049])


				# Corridor Ogre
				lnk(xx = 507, yy = 600, name_id = [14448])
				loot_items(xx = 507, yy = 600, loot_source_name = [14448, 1049], item_autoconvert_list = [4121, 4118, 4113, 4116, 5005, 5098])
			
				# Bone Corridor Undead
				lnk(xx = 497, yy = 519, name_id = [14107, 14081, 14082])
				lnk(xx = 467, yy = 519, name_id = [14083, 14107, 14081, 14082])
				loot_items(xx = 507, yy = 600, loot_source_name = [14107, 14081, 14082])

				# Wonnilon Undead
				lnk(xx = 536, yy = 414, name_id = [14127, 14126, 14125, 14124, 14092, 14123])
				lnk(xx = 536, yy = 444, name_id = [14127, 14126, 14125, 14124, 14092, 14123])


				# Huge Viper
				lnk(xx = 550, yy = 494, name_id = [14088])

				# Ogre + Goblins
				lnk(xx = 565, yy = 508, name_id = [14185, 14186, 14187, 14448])
				lnk(xx = 565, yy = 494, name_id = [14185, 14186, 14187, 14448])
				loot_items(xx = 565, yy = 508, loot_source_name = [14185, 14186, 14187, 14448])

				# Ghasts near prisoners
				lnk(xx = 545, yy = 553, name_id = [14128, 14129, 14136, 14095, 14137, 14135])
				loot_items(xx = 545, yy = 553, loot_source_name = [1040])

				# Black Widow Spiders
				lnk(xx = 440, yy = 395, name_id = [14417])


				# NW Ghast room near hideout
				lnk(xx = 390, yy = 390, name_id = [14128, 14129, 14136, 14095, 14137, 14135])


				if get_v('qs_autokill_temple_level_1_stage') == 0:
					set_v('qs_autokill_temple_level_1_stage', 1)
					
				elif get_v('qs_autokill_temple_level_1_stage') == 1:
					set_v('qs_autokill_temple_level_1_stage', 2)

					# Gnoll & Bugbear southern room
					lnk(xx = 515, yy = 535, name_id = [14078, 14249, 14066, 14632, 14164])
					lnk(xx = 515, yy = 549, name_id = [14067, 14631, 14078, 14249, 14066, 14632, 14164])
					loot_items(xx = 515, yy = 540)

					# Gnoll & Bugbear northern room
					lnk(xx = 463, yy = 535, name_id = [14248, 14631, 14188, 14636, 14083, 14184, 14078, 14249, 14066, 14632, 14164])
					loot_items(xx = 463, yy = 535)

					# Earth Temple Fighter eastern room
					lnk(xx = 438, yy = 505, name_id = [14337, 14338])
					loot_items(xx = 438, yy = 505, item_autoconvert_list = [6074, 6077, 5005, 4123, 4134])

					# Bugbear Central Outpost
					lnk(xx = 505, yy = 476, name_id = [14165, 14163, 14164, 14162])
					loot_items(xx = 505, yy = 476)

					# Bugbears nea r Wonnilon
					lnk(xx = 555, yy = 436, name_id = [14165, 14163, 14164, 14162])
					lnk(xx = 555, yy = 410, name_id = [14165, 14163, 14164, 14162])
					lnk(xx = 519, yy = 416, name_id = [14165, 14163, 14164, 14162])

					loot_items(xx = 519, yy = 416, loot_source_name = range(14162, 14166), item_autoconvert_list = [6174])
					loot_items(xx = 555, yy = 436, loot_source_name = [14164], item_autoconvert_list = [6174])
					loot_items(xx = 555, yy = 410, loot_source_name = [14164], item_autoconvert_list = [6174])

					# Bugbears North of Romag
					lnk(xx = 416, yy = 430, name_id = range(14162, 14166) )
					loot_items(xx = 416, yy = 430, loot_source_name = range(14162, 14166), item_autoconvert_list = [6174])

				elif get_v('qs_autokill_temple_level_1_stage') == 2:
					# Jailer room
					lnk(xx = 568, yy = 462, name_id = [14165, 14164, 14229])
					loot_items(xx = 568, yy = 462, item_autoconvert_list = [6174])
					# Earth Altar
					lnk(xx = 474, yy = 396, name_id = [14381, 14337])
					lnk(xx = 494, yy = 396, name_id = [14381, 14337])
					lnk(xx = 484, yy = 423, name_id = [14296])
					loot_items(xx = 480, yy = 400, loot_source_name = range(1041, 1045), item_proto_list = [6082, 12228, 12031] , item_autoconvert_list = [4070, 4193, 6056, 8025])
					loot_items(xx = 480, yy = 400, item_proto_list = [6082, 12228, 12031] , item_autoconvert_list = [4070, 4193, 6056, 8025])


					# Troop Commander room
					lnk(xx = 465, yy = 477, name_id = ( range(14162, 14166)+ [14337, 14156, 14339]) )
					lnk(xx = 450, yy = 477, name_id = ( range(14162, 14166)+ [14337, 14156, 14339]) )
					loot_items(xx = 450, yy = 476, item_autoconvert_list = [4098, 6074, 6077, 6174])


					# Romag Room
					lnk(xx = 441, yy = 442, name_id = ([8045, 14154] + range(14162, 14166)+ [14337, 14156, 14339]) )
					loot_items(xx = 441, yy = 442, item_autoconvert_list = [6164, 9359, 8907, 9011], item_proto_list = [10006, 6094, 4109, 8008])
					

	if (cur_map == 5003): # Temple Level 2 - Water, Fire & Air Floor
		if get_f('qs_autokill_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Kelno regroup
				lnk(xx = 480, yy = 494, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				lnk(xx = 490, yy = 494, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				lnk(xx = 490, yy = 514, name_id = [8092, 14380, 14292, 14067, 14078, 14079, 14080, 14184, 14187, 14215, 14216, 14275, 14159, 14160, 14161, 14158])
				loot_items(xx = 480, yy = 494, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])
				loot_items(xx = 480, yy = 514, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])
				loot_items(xx = 490, yy = 514, item_proto_list = [10009, 6085, 4219], item_autoconvert_list = [6049, 4109, 6166, 4112])

				# Corridor Ogres
				lnk(xx = 480, yy = 452, name_id = [14249, 14353])
				loot_items(xx = 480, yy = 452, item_autoconvert_list = [4134])

				# Minotaur
				for m_stat in game.obj_list_vicinity(location_from_axis(566, 408), OLC_SCENERY):
					if m_stat.name == 1615:
						m_stat.destroy()
						cnk(14241)
						loot_items(xx = 566, yy = 408)

				# Greater Temple Guards
				lnk(xx = 533, yy = 398, name_id = [14349, 14348])
				lnk(xx = 550, yy = 422, name_id = [14349, 14348])
				loot_items(xx = 533, yy = 398)

				# Littlest Troll
				lnk(xx = 471, yy = 425, name_id = [14350])
				# Carrion Crawler
				lnk(xx = 451, yy = 424, name_id = [14190])


				# Fire Temple Bugbears Outside
				lnk(xx = 397, yy = 460, name_id = [14169])
				loot_items(xx = 397, yy = 460, loot_source_name = [14169])


				if get_v('qs_autokill_temple_level_2_stage') == 0:
					set_v('qs_autokill_temple_level_2_stage', 1)
					
				elif get_v('qs_autokill_temple_level_2_stage') == 1:
					set_v('qs_autokill_temple_level_2_stage', 2)

					# Feldrin
					lnk(xx = 562, yy = 438, name_id = [14311, 14312, 14314, 8041, 14253])
					loot_items(xx = 562, yy = 438, item_proto_list = [6083, 10010, 4082, 6086, 8010], item_autoconvert_list = [6091, 4070, 4117, 4114, 4062, 9426, 8014])

					# Prisoner Guards - Ogre + Greater Temple Bugbear
					lnk(xx = 410, yy = 440, name_id = [8065])
					loot_items(xx = 410, yy = 440, loot_source_name = [8065])

				elif get_v('qs_autokill_temple_level_2_stage') == 2:
					set_v('qs_autokill_temple_level_2_stage', 3)

					# Water Temple
					lnk(xx = 541, yy = 573, name_id = [14375, 14231, 8091, 14247, 8028, 8027, 14181, 14046, 14239, 14225])
					# Juggernaut
					lnk(xx = 541, yy = 573, name_id = [14244])
					loot_items(xx = 541, yy = 573, item_proto_list = [10008, 6104, 4124, 6105, 9327, 9178], item_autoconvert_list = [6039, 9508, 9400, 6178, 6170, 9546, 9038, 9536])

					# Oohlgrist
					lnk(xx = 483, yy = 614, name_id = [14262, 14195])
					loot_items(xx = 483, yy = 614, item_proto_list = [6101, 6107], item_autoconvert_list = [6106, 12014, 6108])

					# Salamanders
					lnk(xx = 433, yy = 583, name_id = [8063, 14384, 14111])
					lnk(xx = 423, yy = 583, name_id = [8063, 14384, 14111])
					loot_items(xx = 433, yy = 583, item_proto_list = [4028, 12016, 6101, 4136], item_autoconvert_list = [6121, 8020])

				elif get_v('qs_autokill_temple_level_2_stage') == 3:
					set_v('qs_autokill_temple_level_2_stage', 4)

					# Alrrem
					lnk(xx = 415, yy = 499, name_id = [14169, 14211, 8047, 14168, 14212, 14167, 14166, 14344, 14224, 14343])
					loot_items(xx = 415, yy = 499, item_proto_list = [10007, 4079, 6082], item_autoconvert_list = [6094, 6060, 6062, 6068, 6069, 6335, 6269, 6074, 6077, 6093, 6167, 6177, 6172, 8019, 6039, 4131, 6050, 4077, 6311])

				elif get_v('qs_autokill_temple_level_2_stage') == 4:
					set_v('qs_autokill_temple_level_2_stage', 5)
					# Big Bugbear Room
					lnk(xx = 430, yy = 361, name_id = (range(14174, 14178) +[14213, 14214, 14215, 14216])  )
					lnk(xx = 430, yy = 391, name_id = (range(14174, 14178) +[14213, 14214, 14215, 14216])  )
					loot_items(xx = 430, yy = 361, item_autoconvert_list = [6093, 6173, 6168, 6163, 6056])
					loot_items(xx = 430, yy = 391, item_autoconvert_list = [6093, 6173, 6168, 6163, 6056])


	if (cur_map == 5105): # Temple Level 3 - Thrommel Floor
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Northern Trolls
				lnk(xx = 394, yy = 401, name_id = [14262])

				# Shadows
				lnk(xx = 369, yy = 431, name_id = [14289])
				lnk(xx = 369, yy = 451, name_id = [14289])

				# Ogres:
				lnk(xx = 384, yy = 465, name_id = [14249])
				loot_items(xx = 384, yy = 465)

				# Ettin:
				lnk(xx = 437, yy =524, name_id = [14238])
				loot_items(xx = 437, yy = 524)

				# Yellow Molds:
				lnk(xx = 407, yy =564, name_id = [14276])

				# Groaning Spirit:
				lnk(xx = 441, yy = 459, name_id = [14280])
				loot_items(xx = 441, yy = 459, item_proto_list = [4218, 6090], item_autoconvert_list = [9214, 4191, 6058, 9123, 6214, 9492, 9391, 4002])

				# Key Trolls:
				lnk(xx = 489, yy = 535, name_id = [14262])
				lnk(xx = 489, yy = 504, name_id = [14262])
				loot_items(xx = 489, yy = 504, item_proto_list = range(10016, 10020) )
				loot_items(xx = 489, yy = 535, item_proto_list = range(10016, 10020) )

				# Will o Wisps:
				lnk(xx = 551, yy = 583, name_id = [14291])

				# Lamia:
				lnk(xx = 584, yy = 594, name_id = [14342, 14274])
				loot_items(xx = 584, yy = 594, item_proto_list = [4083])

				# Jackals, Werejackals & Gargoyles:
				lnk(xx = 511, yy = 578, name_id = [14051, 14239, 14138])
				lnk(xx = 528, yy = 556, name_id = [14051, 14239, 14138])

				# UmberHulks
				lnk(xx = 466, yy = 565, name_id = [14260])

				if get_v('qs_autokill_temple_level_3_stage') == 0:
					set_v('qs_autokill_temple_level_3_stage', 1)
					
				elif get_v('qs_autokill_temple_level_3_stage') == 1:
					set_v('qs_autokill_temple_level_3_stage', 2)
				
					# Gel Cube
					lnk(xx = 476, yy = 478, name_id = [14139])

					# Black Pudding
					lnk(xx = 442, yy = 384, name_id = [14143])

					# Goblins:
					lnk(xx = 491, yy = 389, name_id = (range(14183, 14188)+ [14219, 14217]) )
					loot_items(xx = 491, yy = 389)

					# Carrion Crawler:
					lnk(xx = 524, yy = 401, name_id = [14190] )

					# Ogres near thrommel:
					lnk(xx = 569, yy = 412, name_id = [14249, 14353] )
					loot_items(xx = 569, yy = 412, loot_source_name = [14249, 14353], item_autoconvert_list = [4134])

					# Leucrottas:
					lnk(xx = 405, yy = 590, name_id = [14351] )

				elif get_v('qs_autokill_temple_level_3_stage') == 2:
					set_v('qs_autokill_temple_level_3_stage', 3)

					# Pleasure dome:
					lnk(xx = 553, yy = 492, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 540, yy = 480, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 569, yy = 485, name_id = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177] )



					loot_items(xx = 540, yy = 480, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					loot_items(xx = 553, yy = 492, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					loot_items(xx = 569, yy = 485, loot_source_name = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177], item_autoconvert_list = [6334])
					game.global_flags[164] = 1 # Turns on Bugbears

				elif get_v('qs_autokill_temple_level_3_stage') == 3:
					set_v('qs_autokill_temple_level_3_stage', 4)
					# Pleasure dome - make sure:
					lnk(xx = 553, yy = 492, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 540, yy = 480, name_id = [14346, 14174, 14249, 14176, 14353, 14175, 14352, 14177] )
					lnk(xx = 569, yy = 485, name_id = [8034, 14346, 14249, 14174, 14176, 14353, 14175, 14352, 14177] )


					# Smigmal & Falrinth
					ass1 = game.obj_create(14782, location_from_axis(614, 455) )
					ass2 = game.obj_create(14783, location_from_axis(614, 455) )
					lnk(xx = 614, yy = 455, name_id = [14232, 14782, 14783] )
					loot_items(xx = 614, yy = 455, item_proto_list = [10011, 6125, 6088], item_autoconvert_list = [4126, 6073, 6335, 8025])

					lnk(xx = 614, yy = 480, name_id = [14110, 14177, 14346, 20123] )
					loot_items(xx = 619, yy = 480, item_proto_list = [12560, 10012, 6119], item_autoconvert_list = [4179, 9173])
					loot_items(xx = 612, yy = 503, loot_source_name = [1033], item_proto_list = [12560, 10012, 6119], item_autoconvert_list = [4179, 9173])


	if (cur_map == 5080): # Temple Level 4 - Greater Temple
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				game.global_flags[820] = 1 # Trap Disabled
				game.global_flags[148] = 1 # Paida Sane
				# Eastern Trolls
				lnk(xx = 452, yy = 552, name_id = [14262])
				# Western Trolls
				lnk(xx = 513, yy = 552, name_id = [14262])

				# Troll + Ettin
				lnk(xx = 522, yy = 586, name_id = [14262, 14238])
				loot_items(xx = 522, yy = 586)

				# Hill Giants
				lnk(xx = 570, yy = 610, name_id = [14218, 14217, 14219])
				loot_items(xx = 570, yy = 610)

				# Ettins
				lnk(xx = 587, yy = 580, name_id = [14238])
				loot_items(xx = 587, yy = 580)

				# More Trolls
				lnk(xx = 555, yy = 546, name_id = [14262])

				if get_v('qs_autokill_temple_level_4_stage') == 0:
					set_v('qs_autokill_temple_level_4_stage', 1)
					
				elif get_v('qs_autokill_temple_level_4_stage') == 1:
					set_v('qs_autokill_temple_level_4_stage', 2)
					# Bugbear quarters
					lnk(xx = 425, yy = 591, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 435, yy = 591, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 434, yy = 603, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])
					lnk(xx = 405, yy = 603, name_id = [14174, 14175, 14176, 14177, 14249, 14347, 14346 ])

					loot_items(xx = 435, yy = 590)
					loot_items(xx = 425, yy = 590)
					loot_items(xx = 435, yy = 603)
					loot_items(xx = 405, yy = 603)

				elif get_v('qs_autokill_temple_level_4_stage') == 2:
					set_v('qs_autokill_temple_level_4_stage', 3)
					# Insane Ogres
					lnk(xx = 386, yy = 584, name_id = [14356, 14355, 14354])
					loot_items(xx = 386, yy = 584)
					# Senshock's Posse
					lnk(xx = 386, yy = 528, name_id = [14296, 14298, 14174, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(386, 528), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 386, yy = 528)


				elif get_v('qs_autokill_temple_level_4_stage') == 3:
					set_v('qs_autokill_temple_level_4_stage', 4)
					# Hedrack's Posse
					lnk(xx = 493, yy = 442, name_id = [14238, 14239, 14218, 14424, 14296, 14298, 14174, 14176, 14177, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(493, 442), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 493, yy = 442)

					lnk(xx = 465, yy = 442, name_id = [14238, 14239, 14218, 14424, 14296, 14298, 14174, 14176, 14177, 14110, 14302, 14292])
					for obj_1 in game.obj_list_vicinity(location_from_axis(493, 442), OLC_NPC):
						for pc_1 in game.party[0].group_list():
							obj_1.ai_shitlist_remove( pc_1 )
							obj_1.reaction_set( pc_1, 50 )
					loot_items(xx = 493, yy = 442)

					# Fungi
					lnk(xx = 480, yy = 375, name_id = [14274, 14143, 14273, 14276, 14142, 14141, 14282])
					loot_items(xx = 484, yy = 374)
					loot_items(xx = 464, yy = 374)

					lnk(xx = 480, yy = 353, name_id = [14277, 14140])





##################################
###  NODES						 #
##################################

	if (cur_map == 5130): # Fire Node
		if get_f('qs_autokill_nodes'):
			# Fire Toads
			lnk(xx = 535, yy = 525, name_id = [14300])
			
			# Bodaks
			lnk(xx = 540, yy = 568, name_id = [14328])
			
			# Salamanders
			lnk(xx = 430, yy = 557, name_id = [14111])
			
			# Salamanders near Balor
			lnk(xx = 465, yy = 447, name_id = [14111])			
			
			# Efreeti
			lnk(xx = 449, yy = 494, name_id = [14340])

			# Fire Elementals + Snakes
			lnk(xx = 473, yy = 525, name_id = [14298, 14626])
			lnk(xx = 462, yy = 532, name_id = [14298, 14626])

				





		


			




##########################
###  VERBOBONC		 #
##########################

	if (cur_map == 5154): # Scarlett Bro bottom floor
		if get_f('qs_autokill_greater_temple'):
			game.global_flags[984] = 1 # Skip starter convo
			game.global_flags[982] = 1


	if (cur_map == 5152): # Prince Zook quarters
		if get_f('qs_autokill_greater_temple'):
			game.global_flags[969] = 1 # Met prince Zook
			game.global_flags[985] = 1 # Mention Drow Problem
			game.quests[69].state = qs_accepted
			game.global_flags[981] = 1 # Zook said Lerrick mean
			game.global_vars[977] = 1 # Zook said talk to Absalom abt Lerrick
			if game.global_vars[999]  >= 15:
				game.quests[69].state = qs_completed
			

	if (cur_map == 5041): # Drow Caves I - spidersfest
		if get_f('qs_autokill_greater_temple'):
			
			# Spidors 1
			lnk(xx = 465, yy = 471, name_id = [14399, 14397])
			lnk(xx = 451, yy = 491, name_id = [14399, 14397])
			lnk(xx = 471, yy = 491, name_id = [14399, 14397])

			lnk(xx = 437, yy = 485, name_id = [14741, 14397])
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				# Key
				loot_items(item_proto_list = [10022], loot_money_and_jewels_also = 0)

		return



	if (cur_map == 5015): # Drow Caves II - 2nd spidersfest
		if get_f('qs_autokill_greater_temple'):			

			# Spiders
			lnk(xx = 488, yy = 477, name_id = [14741, 14397, 14620])

			# Drow
			lnk(xx = 455, yy = 485, name_id = [14708, 14737, 14736, 14735])
			loot_items(xx = 455, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073])

	if (cur_map == 5019): # Drow Caves III - Drowfest I
		if get_f('qs_autokill_greater_temple'):			

			# Garg. Spider
			lnk(xx = 497, yy = 486, name_id = [14524])

			# Drow
			lnk(xx = 473, yy = 475, name_id = [14399, 14708, 14737, 14736, 14735])
			lnk(xx = 463, yy = 485, name_id = [14399, 14708, 14737, 14736, 14735])
			loot_items(xx = 475, yy = 471, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073])

			lnk(xx = 456, yy = 487, name_id = [14399, 14708, 14737, 14736, 14735, 14734])
			lnk(xx = 427, yy = 487, name_id = [14399, 14708, 14737, 14736, 14735, 14734])
			loot_items(xx = 465, yy = 486, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])
			loot_items(xx = 425, yy = 481, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])
			loot_items(xx = 475, yy = 471, item_autoconvert_list = [4132, 6057, 4082, 4208, 6076, 6046, 6045, 5011, 6040, 6041, 6120, 4193, 6160, 6161, 6334, 4081, 6223, 6073, 6058])

			loot_items(xx = 425, yy = 481, item_proto_list = [6051, 4139, 4137] )

	if (cur_map == 5040): # Drow Caves IV - Spiders cont'd
		if get_f('qs_autokill_greater_temple'):			
			lnk(xx = 477, yy = 464, name_id = [14524, 14399, 14397])
			lnk(xx = 497, yy = 454, name_id = [14524, 14399, 14397])
			lnk(xx = 467, yy = 474, name_id = [14524, 14399, 14397, 14741])
			lnk(xx = 469, yy = 485, name_id = [14524, 14399, 14397])


	if (cur_map == 5006): # Drow Caves V - Young White Dragons
		if get_f('qs_autokill_greater_temple'):	
			lnk(xx = 489, yy = 455, name_id = [14707])

	if (cur_map == 5043): # Drow Caves VI - Adult White Dragon
		if get_f('qs_autokill_greater_temple'):	
			lnk(xx = 480, yy = 535, name_id = [14999])
			loot_items(xx = 480, yy = 535)


	if (cur_map == 5148): # Verbobonc Jail
		if get_f('qs_autokill_greater_temple'):	
			game.quests[79].state = qs_accepted
			game.quests[80].state = qs_accepted
			game.quests[81].state = qs_accepted
			if game.global_vars[964] == 0:
				game.global_vars[964] = 1
			if game.global_flags[956] == 1:
				game.quests[79].state = qs_completed
			if game.global_flags[957] == 1:
				game.quests[80].state = qs_completed
			if game.global_flags[958] == 1:
				game.quests[81].state = qs_completed

	if (cur_map == 5151): # Verbobonc Great Hall
		if get_f('qs_autokill_greater_temple'):	
			game.global_vars[979] = 2 # Allows meeting with Mayor
			game.global_flags[980] = 1 # Got info about Verbobonc

	if (cur_map == 5014): # Spruce Goose Inn
		if get_f('qs_autokill_greater_temple'):
			if is_timed_autokill == 0:
				game.timevent_add(autokill, (cur_map, 1, 1), 100)
			else:
				lnk(xx=484, yy = 479, name_id = 8716) # Guntur Gladstone
				game.global_vars[961] = 2 # Have discussed wreaking havoc
				loot_items(loot_source_name = 8716, item_autoconvert_list = [6202, 6306, 4126, 4161])




	return





#######################
#######################
### END OF AUTOKILL ###
#######################
#######################



from toee import *
from utilities import *
from Co8 import *

##  Writen By Cerulean the Blue
# Modified by Sitra Achara 04-2011

# Miniature Chest internal flags:
# obj_f_item_pad_i_2 - miniature chest ID
# obj_f_item_pad_i_3 - indicates whether the chest can be summoned ("chest is in the Ethereal Plane (1) or in the Prime Plane (0) ")
# obj_f_item_pad_i_4 - Map # where the chest was summoned

def OnBeginSpellCast( spell ):
	print " Extraplanar Chest OnBeginSpellCast"
	print "spell.target_list=", spell.target_list
	print "spell.caster=", spell.caster, " caster.level= ", spell.caster_level
	game.particles( "sp-conjuration-conjure", spell.caster )

def	OnSpellEffect ( spell ):

	print " Extraplanar Chest OnSpellEffect"
	spell.duration = 1
	bgfilename = 'modules\\ToEE\\Bag_of_Holding.mes' 
	proto = 1113
	
	mini = spell.caster.item_find(12105)
	
	if mini == OBJ_HANDLE_NULL:	# Caster hasn't used spell before.  Create miniature chest and subtract 5050 gold from caster
		
		if spell.caster.money_get() >= 505000:
			mini = create_item_in_inventory( 12105, spell.caster )
			set_flag(mini, 0) # sets flag for chest on the Prime Material Plane
			mini.item_flag_set( OIF_IDENTIFIED ) # Makes the mini identified.
			spell.caster.money_adj(-505000)
			chest = game.obj_create(proto, spell.caster.location)
			create_item_in_inventory(11300, chest) # Create Leomund's Secret Chest note in new chest
			#game.particles( 'Orb-Summon-Balor', chest )
			bagnum = boh_newbag() # boh-newbag() is in Co8.py
			Set_ID(mini, chest, bagnum)
		else:
			game.particles( 'Fizzle', spell.caster )
		
	elif (Get_ID_Mini(mini) == 0): # Miniature found but never used before

		
		set_flag(mini, 0) # sets flag for chest on the Prime Material Plane

		
		bagnum = boh_newbag() # boh-newbag() is in Co8.py
		chest = game.obj_create(proto, spell.caster.location)
		create_item_in_inventory(11300, chest) # Create Leomund's Secret Chest note in new chest
		#game.particles( 'Orb-Summon-Balor', chest ) # causes crashes with saddlebags
		Set_ID(mini, chest, bagnum)
		set_mini_map(mini, chest) # record map where the chest was summoned - for forgetful players		
	else:
		
		# Mini found and has been used before.
		chest_here = 0  # flag for whether or not the right chest is in the casters vicinity
		for chest in game.obj_list_vicinity( spell.caster.location, OLC_CONTAINER ):
			# Find correct chest for that mini
			if (chest.name == 1113 and Compare_ID(mini, chest)):
				chest_here = 1
				cxx, cyy = location_to_axis(chest.location)
				#game.particles( 'Orb-Summon-Balor', chest )
				allBagDict = readMes(bgfilename) # readMes is in Co8.py.
				bagnum = boh_newbag() # boh-newbag() is in Co8.py
				Set_ID_Mini(mini, bagnum)
				contents = GetContents(chest)
				allBagDict[bagnum] = contents
				writeMes(bgfilename, allBagDict) # writeMes is in Co8.py
				set_flag(mini, 1)  # Sets fkag flag for chest on Ethereal Plane
				
				# Time event added for chest destruction to allow time for game particles to fire
				Timed_Destroy(chest, 500) # 500 = 1/2 second
			
		if ( not (chest_here) and get_flag(mini) ): # Chest not on this plane:  create chest and fill it.
			chest = game.obj_create(proto, spell.caster.location)
			bagnum = Get_ID_Mini(mini)
			Set_ID(mini, chest, bagnum)
			set_flag(mini, 0) # sets flag for chest on the Prime Material Plane
			set_mini_map(mini, chest) # record map where the chest was summoned - for forgetful players
			
			#game.particles( 'Orb-Summon-Balor', chest )
			contents = boh_getContents(bagnum) # boh_getContents is in Co8.py
			Create_Contents(contents, chest)

		elif ( not (chest_here) and not get_flag(mini) ):
			miniature_chest_map_number = get_mini_map(mini) # retrieve map where the chest was summoned - for forgetful players
			


			spell.caster.float_mesfile_line( 'mes\\spell.mes', 16015, 1 ) # "Chest left at:"
			spell.caster.float_mesfile_line( 'mes\\map_names.mes', miniature_chest_map_number, 1 ) 
			spell.caster.float_mesfile_line( 'mes\\map_numbers.mes', miniature_chest_map_number, 1 ) 			
			# failsaife:
			# if you are on the same map, and your X,Y coordinates are close enough to where the chest was supposed to be, yet no chest was found, then it's probably a bug - reset the miniature to "Ethereal Plane"
			# likewise, if for some reason the chest has no recorded X,Y at all, allow respawning it (mainly catering to r0gershrubber here :) )			
			pxx, pyy = location_to_axis(spell.caster.location)
			cxx, cyy = get_mini_xy(mini)			
			if ( ((pxx-cxx)**2)  + ( (pyy-cyy)**2 )< 81 or cxx == 0  ) and game.leader.map == miniature_chest_map_number: 
				set_flag(mini, 1)
				# error try again message
				spell.caster.float_mesfile_line( 'mes\\spell.mes', 16017, 1 ) # "Failsafe activated!"
				spell.caster.float_mesfile_line( 'mes\\spell.mes', 16018, 1 ) # "Try again."
			elif miniature_chest_map_number in range(5070, 5079):
				# lastly, if you lost it in the wilds, here's your second chance (exploitable, but it's easy enough to cheat in this game anyway)
				spell.caster.float_mesfile_line( 'mes\\spell.mes', 16016, 1 ) # "LOST IN WILDERNESS!"
				if not game.leader.map == miniature_chest_map_number:
					set_flag(mini, 1)
					spell.caster.float_mesfile_line( 'mes\\spell.mes', 16017, 1 ) # "Failsafe activated!"
					spell.caster.float_mesfile_line( 'mes\\spell.mes', 16018, 1 ) # "Try again."					
					



			game.particles( 'Fizzle', spell.caster )
			
		else:
			game.particles( 'Fizzle', spell.caster )
	

	End_Spell(spell)
	spell.spell_end(spell.id, 1)  # fix - adding the endDespiteTargetList flag to force the spell_end and prevent the spell trigger from going on indefinitely


def OnBeginRound( spell ):
	print " Extraplanar Chest OnBeginRound"
	return

def OnEndSpellCast( spell ):
	print " Extraplanar Chest OnEndSpellCast"


	
def GetContents(chest):
	# Gets contents of the chest by proto number and adds them to an ordered list.  The list format is dynamic.
	# Reads Exclude_List.mes.  Items in the exclude list are not preserved in the contents of the chest.
	# This is to prevent charged magic items from being recharged by being in the chest.  Such items are lost if sent to the Ethereal Plane in the chest.
	#  Exclued list is not decoded because we only care about the dictionary keys (proto numbers).  There is no need to decode the descriptions in the dictionary entries.
	ExcludeDict = readMes('modules\\ToEE\\Exclude_List.mes')
	exclude_list = ExcludeDict.keys()
	contents = []
	# Look for proto number 4000-12999.  These are the proto numbers for items that could be in the chest.
	num = 4000 
	while num <= 12999 :
		# Check if proto number is on the exclude list
		if num not in exclude_list:
			item = chest.item_find_by_proto(num)
			# Loop finding item to check for multiple occurences of the same item
			while (item != OBJ_HANDLE_NULL):
				# add the item to the list of contents
				contents.append(num)
				# check if item is stackable, and if so get the quantity stacked
				quantity = 0
				type = item.type
				if (type == obj_t_ammo):
					quantity = item.obj_get_int(obj_f_ammo_quantity)
				else:
					quantity = item.obj_get_int(obj_f_item_quantity)
				# if there is more than one in the stack, add the quantity to the contents list.  Max quantity 3999 to keep quantity from matching any proto number in the list.
				if ( quantity > 1 ):
					if quantity >= 4000:
						quantity = 3999
					contents.append(quantity)
				# check to see if item is identified.  If so, add the identified flag 1 to the list.
				FLAGS = item.item_flags_get()
				if (FLAGS & OIF_IDENTIFIED):
					contents.append(1)
				# destroy the item and check if there is another with the same proto number
				item.destroy()
				item = chest.item_find_by_proto(num)
		num += 1
	# add "end of list" number to the end of the list.
	contents.append(99999)
	return contents

def Create_Contents(contents, chest):
	# Recreates the contents of the chest from the ordered list.
	# Uses a "while" statement rather than a "for" statement in order to be able to step through the dynamic list.
	i = 0
	while i in range(len(contents)):
		# check to make sure we are looking at a proto number and not a quantity, identified flag, or end of list marker.
		if (contents[i] >= 4000 and contents[i] != 99999):
			#create item in chest
			item = create_item_in_inventory( contents[i], chest )
			# step to the next number on the list
			i += 1
			if i in range(len(contents)): # probably not necessary here, but keeps us safe.
				# check to see if next number on the list is a quantity or identified flag
				if contents[i] < 4000:
					quantity = contents[i]
					# check if item is ammo
					if item.type == obj_t_ammo:
						# check if "quantity" is actually a quantity and not an identified flag
						if quantity > 1:
							# "quantity" is a quantity,  Set item quantity and step to next number on the list
							item.obj_set_int(obj_f_ammo_quantity, quantity)
							i += 1
						else:
							# "quantity" is an identified flag.  Set item quantity to 1.
							item.obj_set_int(obj_f_ammo_quantity, 1)
					# check if item is a potion, scroll or other stackable item.
					else:
						# check if "quantity" is actually a quantity and not an identified flag
						if quantity > 1:
							# "quantity" is a quantity,  Set item quantity and step to next number on the list
							item.obj_set_int(obj_f_item_quantity, quantity)
							i += 1
						else:
							# "quantity" is an identified flag.  Set item quantity to 1.
							item.obj_set_int(obj_f_item_quantity, 1)
			if i in range(len(contents)): # is necessary here
				# check if contents[i] is an identified flag.
				if contents[i] == 1:
					# flag item as identified and step to next number on the list.
					item.item_flag_set( OIF_IDENTIFIED )
					i += 1
		else:
			i += 1
	return

def Set_ID(mini, chest, num): # Generates a random number and sets a field in the item, chest and caster to that number.
	# ID_number = game.random_range( 1,2147483647 )
	# ID_number = ID_number^game.random_range( 1,2147483647 )#xor with next "random" number in line, should be more random
	mini.obj_set_int(obj_f_item_pad_i_2, num)
	chest.obj_set_int(obj_f_container_pad_i_1, num)
	return num

def Set_ID_Mini(mini, num):
	mini.obj_set_int(obj_f_item_pad_i_2, num)
	return mini.obj_get_int(obj_f_item_pad_i_2)

def Get_ID_Mini(mini): # Reads the ID number of the miniature chest.
	return mini.obj_get_int(obj_f_item_pad_i_2)

def Compare_ID(mini, chest): # Compares the ID number of the large chest and the miniature chest.  Returns 1 if they match, otherwise returns o.
	if (mini.obj_get_int(obj_f_item_pad_i_2) == chest.obj_get_int(obj_f_container_pad_i_1)):
		return 1
	else:
		return 0

def set_flag(mini, x): # Store a flag in a field of the miniature chest.  1 means on the Ethereal Plane, 0 means on the Prime Material Plane.
	mini.obj_set_int( obj_f_item_pad_i_3, x )
	return mini.obj_get_int( obj_f_item_pad_i_3 )

def get_flag(mini): # Reads a flag from a field of the miniature chest.
	return mini.obj_get_int( obj_f_item_pad_i_3 )


def set_mini_map(mini, chest):
	cxx, cyy = location_to_axis(chest.location)
	mini.obj_set_int( obj_f_item_pad_i_4, mini.map  )
	mini.obj_set_int( obj_f_item_pad_i_5, (cxx + (cyy << 10) )  )
	return mini.obj_get_int( obj_f_item_pad_i_4  )

def get_mini_map(mini):
	return mini.obj_get_int( obj_f_item_pad_i_4 )

def get_mini_xy(mini):
	return ( mini.obj_get_int( obj_f_item_pad_i_5 )>>10 , mini.obj_get_int( obj_f_item_pad_i_5 ) & ((2**10) -1) )

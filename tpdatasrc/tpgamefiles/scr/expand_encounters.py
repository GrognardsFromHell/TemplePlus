import os
import sys
from toee import *
#name convention for encounterpacks is EncounterPack + Creator name + pack number
#the file must contain a function called EncounterExpander which takes 2 parameters and returns a list of RE_expanded to be added

sys.path.insert(0, '.\encounterpacks')
encounterpacks = os.listdir('.\encounterpacks')	#to install a new encounterpack simply create a folder called encounterpacks in your main TOEE folder and save the file there

class RE_expanded:
	# Description:

	# RE_expanded is an object that is initialized in the following format:
	# RE_expanded( base DC,( (proto, min, max), (proto2, min, max, [optional - DC increment for each one of proto2])),encounter ID (no idea how it's used?), maps to be used)
	# You may then use the method get_enemies() to get a normal list of enemies, as the game expects.
	# It's more convenient than handling a bunch of 'ifs'.
	# Note that the number of protos is not limited, and you can add the optional DC modifier to any of them
	# e.g.
	# RE_expanded( 2 , ( (14053, 1, 3, 0.5), (14052, 2, 2) ), 1000, [5070,5071] )
	# returns an object RE_expanded with the above proto lists, DC etc.

	# usage:
	# For every RE expanded you want, return a list with the RE_expanded you want to add in your encounterpack
	# Then - the script randomly selects from the list, and uses the get_enemies method to return the final list
	
	enemies = ()
	dc = 0
	maplist = []
	def __init__(self, dc_in = 1, enemy_in = ( (14070, 1 , 1 , 0.5), ), id_in = 1000, maplist_in = [5070,5071,5072,5073,5074,5075,5076,5077]):
		self.dc_base = dc_in
		self.enemy_pool = enemy_in
		self.id = id_in
		self.maplist.extend(maplist_in)
	def get_enemies(self):
		enemy_list_output = ()
		dc_mod = 0.0
		for tup_x in self.enemy_pool:
			nn = game.random_range( tup_x[1], tup_x[2] )
			if nn > 0:
				enemy_list_output += ( ( tup_x[0], nn ), )
				if len(tup_x) == 4: # DC modifier
					dc_mod += tup_x[3] * nn
				
		self.enemies = enemy_list_output
		self.dc = self.dc_base + int(dc_mod)
		return enemy_list_output

	
def encounter_coordinator( setup, encounter ):
	if((1000 <= encounter.id <= 1999 ) or encounter.id == 4000):
		extra_encounters(setup, encounter)											
	return
		
def extra_encounters(setup, encounter):
	expanded_list = []
	nobe = 8			#average number of base encounters, in vanilla TOEE the average number of encounters per map is 8.375, rounded down to 8
	if game.global_flags[500] == 1 and encounter.id == 4000:
		nobe = 5		#average number of sleep encounters for vanilla TOEE, rounded up from 4,81
	elif game.global_flags[500] == 1:
		nobe = 14		#average number of random encounters for Co8 NC TOEE, rounded up from 13,75
	elif encounter.id == 4000:
		nobe = 6		#average number of sleep encounters for Co8 NC TOEE rounded up from 5.642
	
						#the average number of encounters per sleep area varies greatly, it could be improved with an area by area number, defaulting to the average if not found
	
	for f in encounterpacks:
		print (f)
		if f.startswith('EncounterPack'):																	#name convention is EncounterPack + Creator name + pack number
			expanded_list.extend((__import__(os.path.splitext(f)[0]).EncounterExpander(setup, encounter)))	#search encounterpacks and receive returned encounters	            
																											#encounter and setup provided for customization
																											
	if(len (expanded_list) > 0 and game.random_range(1,len(expanded_list ) + nobe) <= len(expanded_list)):	#weighted chance to find a new encounter instead of a base one, since this code is only called when
																					#the game found a normal random encounter, here we make a random check, with a weighted chance that 
																					#depends on how	many encounters we added and how many base encounters there are. This way even though
																					#we added new encounters, there will be a fair chance for all encounters, base and extra, to appear.
		
		aaa = game.random_range(0, len(expanded_list) -1)											#NC base encounters average to 13,75~, rounded to 14	
		encounter.enemies = expanded_list[aaa].get_enemies()
		encounter.dc = expanded_list[aaa].dc													#encounter dc, id, enemies, location (player spawnpoint)

		if(encounter.id != 4000):
			if(len(expanded_list[aaa].maplist) > 0):
				encounter.map = expanded_list[aaa].maplist[game.random_range(0, len(expanded_list[aaa].maplist) -1)]
			randomLocation(encounter)
			encounter.id = expanded_list[aaa].id

		elif(encounter.id == 4000):
			encounter.map = game.leader.map

	return

def randomLocation(encounter):
	r = game.random_range(1,3)
	if (r == 1):
		encounter.location = location_from_axis( 470, 480 )
	elif (r == 2):
		encounter.location = location_from_axis( 503, 478 )
	else:
		encounter.location = location_from_axis( 485, 485 )
	return

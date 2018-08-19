import os
import random
from toee import *
#name convention for encounterpacks is EncounterPack + Creator name + pack number
#the file must contain a function called EncounterExpander which takes 2 parameters and returns a list of RE_expanded to be added


encounterpacks = os.listdir('.\overrides\scr')	#needs to be changed, not sure which folder should contain these files

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
	if((1000 <= encounter.id <= 1999 ) or encounter.id == 4000):	#is it a random encounter? or is it a sleep encounter? if it's neither, don't touch it!
		extra_encounters(setup, encounter)											
	return														
		
def extra_encounters(setup, encounter):
	expanded_list = []
	
	for f in encounterpacks:
		#print (f)						#testing purposes only, to check what files are available in the opened folder
		if f.startswith('EncounterPack'):																	#name convention is EncounterPack + Creator name + pack number
			expanded_list.extend((__import__(os.path.splitext(f)[0]).EncounterExpander(setup, encounter)))	#search encounterpacks and receive returned encounters	            
																											#encounter and setup provided for customization
																											
	if(game.random_range(1,len(expanded_list ) + 14) <= len(expanded_list)):						#weighted chance to find a new encounter instead of a base one	
																									#NC base encounters average to 13,75~, rounded to 14	

		aaa = game.random_range(0, len(expanded_list) -1)											#it's time to determine encounter dc, id, enemies, location (player spawnpoint)
		encounter.enemies = expanded_list[aaa].get_enemies()
		encounter.dc = expanded_list[aaa].dc														

		if(encounter.id != 4000):								#not a sleep encounter, take a map from those available for the encounter and its id, then randomize a location
			if(len(expanded_list[aaa].maplist) > 0):												
				encounter.map = expanded_list[aaa].maplist[game.random_range(0, len(expanded_list[aaa].maplist) -1)]
			randomLocation(encounter)
			encounter.id = expanded_list[aaa].id

		elif(encounter.id == 4000):								#it's a sleep encounter! we shouldn't change the map
			encounter.map = game.leader.map

	return

def randomLocation(encounter):									#taken from Co8 version of random_encounter.py, seems to work fine
	r = game.random_range(1,3)
	if (r == 1):
		encounter.location = location_from_axis( 470, 480 )
	elif (r == 2):
		encounter.location = location_from_axis( 503, 478 )
	else:
		encounter.location = location_from_axis( 485, 485 )
	return
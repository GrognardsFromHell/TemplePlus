from expand_encounters import *


def EncounterExpander(setup, encounter):
	list = []
	list.append(RE_expanded( 1,    ( (14123, 1, 2, 1),  ),      1028, [5070]))  #Test fight with zombies
	list.append(RE_expanded( 1,    ( (14123, 3, 4, 1),  ),      1028))  		#Test fight with zombies, maplist is absent so it's randomized

	return list



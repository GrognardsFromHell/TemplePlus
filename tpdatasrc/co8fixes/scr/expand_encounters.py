from random_encounter import *
import os
	
#name convention for encounterpacks is EncounterPack + Creator name + pack number
#the file must contain a function called EncounterExpander which takes 2 parameters and returns a list of RE_entry to be added

	
def encounter_coordinator( setup, encounter ):
	var = encounter_exists(setup, encounter) 										#original function is called immediately
	if(get_f('qs_is_repeatable_encounter') and game.global_flags[500] == 1):		#checks if the encounter is a random and NC is enabled
		extra_encounters(setup, encounter)											#therefore avoids interrupting scripted and special encounters
	return var						
		
def extra_encounters(setup, encounter):
	expanded_list = []
	
	encounterpacks = os.listdir('.\data\scr')	#correct location for the final release (?)
	for f in encounterpacks:
		if f.startswith('EncounterPack'):																	#name convention is EncounterPack + Creator name + pack number
			expanded_list.extend((__import__(os.path.splitext(f)[0]).EncounterExpander(setup, encounter)))	#search encounterpacks and receive returned encounters	            
																											#encounter and setup provided for customization
																											
	if( game.random_range(1,len(expanded_list ) + 14) <= len(expanded_list)):						#weighted chance to find a new encounter instead of a base one	
		aaa = game.random_range(0, len(expanded_list)-1 )											#NC base encounters average to 13,75~, rounded to 14	
		encounter.enemies = expanded_list[aaa].get_enemies()
		encounter.dc = expanded_list[aaa].dc
		encounter.id = expanded_list[aaa].id	
	return 0



def craft_wand(item, crafter):
	game.global_vars[111] = 444 # test!
	return 1
	
def craft_wand_new_name(item, crafter, spellLevel = -1):
	itemDescr = (item.description).lower()
	
	if spellLevel <= 1:
		return item.description +  ", CL1"
	
	return item.description + ", CL" + str(spellLevel)
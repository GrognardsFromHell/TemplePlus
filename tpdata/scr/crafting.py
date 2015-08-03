
def craft_wand(item, crafter):
	game.global_vars[111] = 444 # test!
	return 1
	
def craft_wand_new_name(item, crafter, spellLevel = -1):
	itemDescr = item.description
	if itemDescr.lower().find('1st'):
		dammitRanOutOfTime = 1
	if spellLevel <= 0:
		return item.description
	if spellLevel == 1:
		return item.description + ", 1st"
	if spellLevel == 2:
		return item.description + ", 2nd"
	if spellLevel == 3:
		return item.description + ", 3rd"
	if (spellLevel >= 4 and spellLevel <= 9):
		return item.description + ", " + str(spellLevel) +"th"
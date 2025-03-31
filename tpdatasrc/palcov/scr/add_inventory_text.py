def add_inventory_text(lines):
	foundTag = False
	search_string = "#Extended base classes added by temple+ below"

	inv_source_text="""
	
	# Favored Soul starting equipment (50,7,12,25,2,1,1 = 98 gp)   start money = 125 gp
	10034
	100,POTION CURE
	100,(SCALE MAIL,LARGE WOODEN SHIELD,heavy mace,LIGHT CROSSBOW,BOOTS,GLOVES)

	# Scout starting equipment (25,10,30,1,1,30 = 97 gp)   start money = 125 gp
	10046
	100,POTION CURE
	100,(studded leather armor,shortsword,shortbow,quiver of arrows,green cloak,black leather boots,thieves tools)

	# Warmage starting equipment (10,3,5,25,1 = 44 gp)   start money = 100 gp
	10047
	100,POTION CURE
	100,(LEATHER ARMOR,SMALL SHIELD,light mace,LIGHT CROSSBOW,BOOTS)

	# Beguiler starting equipment (10,10,25,2,1,1,30 = 79 gp)   start money = 150 gp
	10048
	100,POTION CURE
	100,(LEATHER ARMOR,shortsword,LIGHT CROSSBOW,BOOTS,black leather gloves,thieves tools)

	# Swashbuckler starting equipment (25,20,30,1,1,1 = 78gp)   start money = 150 gp
	10049
	100,POTION CURE
	100,(studded leather armor,rapier,shortbow,quiver of arrows,red bandana,black leather boots)

	"""
	
	new_lines = inv_source_text.split("\n")
	lines.extend(new_lines)
	return new_lines
		
		
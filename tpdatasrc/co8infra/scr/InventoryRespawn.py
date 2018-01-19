################################################
##  Writen by Cerulean the Blue
################################################

from utilities import *

from Co8 import *

# Define dictionaries of ordinary gems and jewelry in the game.
# Format is  key : [value in gp, [list of proto numbers]]
gem_table = {
	1: [10, [12042, 12044]],
	2: [50, [12041, 12042]],
	3: [100, [12035, 12040]],
	4: [500, [12034, 12039]],
	5: [1000, [12010, 12038]],
	6: [5000, [12036, 12037]]
	} 

jewelry_table = {
	1: [50, [6180, 6190]],
	2: [100, [6181, 6185]],
	3: [200, [6157]],
	4: [250, [6182, 6194]],
	5: [500, [6186, 6191]],
	6: [750, [6183, 6193]],
	7: [1000, [6184, 6192]],
	8: [2500, [6187, 6197]],
	9: [5000, [6188, 6195]],
	10: [7500, [6189, 6196]]
	}

def RespawnInventory(attachee, num = 0):
# Removes all attachee's inventory, and respawns it friom the InvenSource.mes line number specified by 'num'.
# If num is not given in the function call, the function will attempt to use the default InvenSource.mes line number for the attachee, if one is defined.
# If no InvenSource.mes line number is defined, the function will terminate.
# Example call 1:  RespawnInventory(attachee, 1) will create Burne's inventory(per line number 1 in InvenSource.mes) in attachee's inventory.
# Example call 2:  RespawnInventory(attachee) will attempt to create the attachee's pre-defined inventory (per InvenSource.mes).
# If the attachee has no Inventory Source defined, the function will terminate.
	if num == 0:
		if attachee.type == obj_t_container:
			num = attachee.obj_get_int( obj_f_container_inventory_source)
		elif attachee.type == obj_t_npc:
			num = attachee.obj_get_int(obj_f_critter_inventory_source)
		else:
			print attachee, 'is not a valid type'
			return
	if num == 0:
		print attachee, 'has no inventory source defined'
		print 'Please specify an inventory to respawn'
		return
	ClearInv(attachee)
	CreateInv(attachee, num)
	return

def ClearInv(attachee):
# Removes all inventory from attachee.
	for num in range(4000, 13000):
		item = attachee.item_find_by_proto(num)
		while (item != OBJ_HANDLE_NULL):
			item.destroy()
			item = attachee.item_find_by_proto(num)
	return

def CreateInv(attachee, num):
# Creates inventory from the structured list created by GetInv from the InvenSource.mes line number 'num'.
	inv = GetInv(num)
	for i in range(len(inv)):
		if not (type(inv[i][0]) is str):
			if type(inv[i][1]) is int:
				if inv[i][0] <= 100:
					chance = inv[i][0]
					if chance >= game.random_range(1,100):
						create_item_in_inventory(inv[i][1], attachee)
				else:
					money = create_item_in_inventory(inv[i][0], attachee)
					money.obj_set_int(obj_f_money_quantity, inv[i][1])
			else:
				if inv[i][0] == 100:
					n = game.random_range(0, len(inv[i][1]) - 1)
					create_item_in_inventory(inv[i][1][n], attachee)
				elif inv[i][0] >= 7000 and inv[i][0] <= 7003:
					money = create_item_in_inventory(inv[i][0], attachee)
					money.obj_set_int(obj_f_money_quantity, game.random_range(inv[i][1][0], inv[i][1][1]))
		else:
			gjlist = CalcGJ(inv[i][0], inv[i][1])
			if gjlist != []:
				for k in range(len(gjlist)):
					create_item_in_inventory(gjlist[k], attachee)
	return

def GetInv(num, filename = 'data\\rules\\InvenSource.mes'):
# Reads InvenSource.mes, finds the line numbered 'num', and creates a structured list of the entries in that line.
	InvDict = readMes(filename)  #readMes is in Co8.py
	InvLine = InvDict[num][0]
	InvLine = InvLine.split(':')
	InvLine.remove(InvLine[0])
	InvLine[0] = InvLine[0].strip()
	n = InvLine[0].find('_num')
	if n != -1:
	 	n = n + 7
	 	InvLine[0] = InvLine[0][n:]
	inv = InvLine[0]
	inv = inv.split(' ')
	for i in range(len(inv)):
		if inv[i].find('(') == -1:
			inv[i] = inv[i].split(',')
			for j in range(len(inv[i])):
				if inv[i][j] == 'copper':
					inv[i][j] = 7000
				elif inv[i][j] == 'silver':
					inv[i][j] = 7001
				elif inv[i][j] == 'gold':
					inv[i][j] = 7002
				elif inv[i][j] == 'platinum':
					inv[i][j] = 7003
				elif type(inv[i][j]) is str and inv[i][j].find('-') != -1:
					inv[i][j] = inv[i][j].split('-')
					for k in range(len(inv[i][j])):
						inv[i][j][k] = ConvertToInt(inv[i][j][k])
				if type(inv[i][j]) is str:
					inv[i][j] = ConvertToInt(inv[i][j])
		else:
			temp1 = inv[i]
			temp1 = str(temp1)
			temp1 = temp1[1:-1]
			temp1 = temp1.split(',')
			for n in range(len(temp1)):
				temp1[n] =  ConvertToInt(temp1[n])
			temp2 = [100, temp1]
			inv[i] = temp2
			
	return inv

def ConvertToInt( string ):
	if type(string) is str:
		try:
			string = int(string)
		except:
			if not (string == 'gems' or string == 'jewelry'):
				print 'WARNING: NON-INTEGER FOUND'
				print 'Non-integer found is', string
	else:
		print 'WARNING:  NON-STRING FOUND'
		print 'Non-string found is', string
	return string

def CalcGJ(string, value):
	gjlist = []
	if string == 'gems':
		table = gem_table
	elif string == 'jewelry':
		table = jewelry_table
	else:
		return gjlist
	if not (type(value) is int):
		value = ConvertToInt(value)
		if not (type(value) is int):
			return gjlist
	n = len(table)
	while value >= table[1][0]:
		if table[n][0] <= value:
			gjlist.append(table[n][1][game.random_range(0, len(table[n][1]) - 1)])
			value = value - table[n][0]
		else:
			n = n - 1
	return gjlist	  



# import os
from description import *
from add_inventory_text import * #Added by temple+

#--------------------------------------------------------------
# Reads inv_source.txt into the dictionary inv_source{}
#--------------------------------------------------------------
def read_inv_source():
	print("\n\n===================================================")
	print("READING inv_source.txt INTO DICTIONARY inv_source{}")
	print("===================================================\n")

	global inv_source
	global error_count
	global error_strings

	invensource_list = []
	invensource_key = 0

	# From scr ----> os.getcwd() = C:\_PALADINS COVE\data\scr
	# In game -----> os.getcwd() = C:\_PALADINS COVE

	# if os.getcwd().strip()[-4:] == "\\scr":
		# file_in = file('inv_source.txt','r')
	# else:
		# file_in = file('scr/inv_source.txt','r')
	
	#Modified by temple+
	file_in = file('data/scr/inv_source.txt','r') 
	lines = file_in.readlines()
	lines = add_inventory_text(lines)

	for line in lines:

		line = line.strip()		# remove whitespace at start and end
		if not line:			# empty lines indicate current invensource is complete
			if invensource_key:
				inv_source[invensource_key] = invensource_list
				invensource_key = 0
			continue
		if line[0] == '#':		# ignore comment lines
			continue
		line = line.split('#')[0]	# remove comments 
		line = line.strip()		# remove whitespace at start and end
		line = line.split('  ')		# split the entries by double spaces, line is now a list

		# found a new invensource key, indicated by a lone integer
		if line[0].isdigit() and len(line) == 1:
			invensource_key = int(line[0].strip())
			invensource_list = []
			print ("New invensource key = " + str(invensource_key) +"\n" )
			continue

		# found a table (or maybe an error)
		else:
			table_list = []
			print ("	table = " + str(line))

			# process each entry in the table
			for i in range(len(line)):
				entry = line[i].strip()
				print ("		entry = " + entry)

				# process entries with a group,
				# eg: '90,(light crossbow,quarrel of bolts,black leather gloves)'	
				grp_start = entry.find('(')
				if grp_start >= 0:
					odds = int(entry[0:grp_start-1])
					grp = entry[grp_start+1:-1]
					print ("		entry is group = " + grp)	
					grp=grp.split(',')
					print ("		after split to list = " + str(grp))					
					for t in range(len(grp)):
						s = grp[t].strip()
						proto = get_proto(s)
						if proto:
							grp.append(proto)
						else:
							break
					if proto == 0:
						break
					grp[0:t+1]=[]
					print ("		group = " + str(tuple(grp)) )											
					table_list.append([odds,tuple(grp)])
					
				# process entries with dice rolls, example: '100,gold,2,10,0'
				# process simple entries, example: '75,boots of speed'
				elif entry.count(',') == 4 or entry.count(',') == 1:
					entry = entry.split(',')
					print ("		entry = " + str(entry))					
					entry[0] = int(entry[0])
					s = entry[1].strip()
					proto = get_proto(s)
					if proto:
						entry[1] = proto
					else:
						break
					if len(entry) == 2:
						table_list.append( [entry[0],entry[1]] )
					else:
						table_list.append( [entry[0],entry[1],int(entry[2]),int(entry[3]),int(entry[4])] )
				else:
					proto,s = 0,entry
					break

			# Append the new table list to the invensource list
			if proto == 0:
				print ("	*** BAD PROTO = '" + s + "',  SKIP THIS TABLE ***\n")
				error_count += 1
				error_strings.append(s)
			else:
				invensource_list.append(table_list)
				print ("	" + str(table_list) + "\n")
	file_in.close()


#----------------------------------------------------------------
# Reads inv_tables.txt into the dictionary inv_tables{}
#----------------------------------------------------------------
def read_inv_tables():
	print("\n\n=====================================================")
	print("READING inv_tables.txt INTO DICTIONARY inv_tables{}")
	print("=====================================================\n")

	global inv_tables
	global error_count2
	global error_strings2

	table_list = []
	table_key = 0

	# if os.getcwd().strip()[-4:] == "\\scr":
		# file_in = file('inv_tables.txt','r')
	# else:
		# file_in = file('scr/inv_tables.txt','r')

	file_in = file('data/scr/inv_tables.txt','r') #Modified by temple+

	for line in file_in.readlines():

		line = line.strip()		# remove whitespace at start and end
		if not line:			# empty lines indicate current invensource is complete
			if table_key:
				inv_tables[table_key] = table_list
				print ("	new inv_sourcetable = " + str(inv_tables[table_key]) + "\n" )
				table_key = 0
			continue
		if line[0] == '#':		# ignore comment lines
			continue
		line = line.split('#')[0]	# remove comments 
		line = line.strip()		# remove whitespace at start and end

		# found a new table, indicated by a line with no comma
		if line.find(',') == -1:
			table_key = line.strip()
			print ("New table key = " + table_key +"\n")
			table_list = []
			continue

		# found an entry, process it if there is a valid table_key active
		elif table_key > 0:
			entry = line.strip()
			print ("	entry = " + entry)

			# process an entry with groups, example: '90,(light crossbow,quarrel of bolts,black leather gloves)'	
			grp_start = entry.find('(')
			if grp_start >= 0:
				odds = int(entry[0:grp_start-1])
				grp = entry[grp_start+1:-1]
				print ("		entry is group = " + grp)	
				grp=grp.split(',')
				print ("		after split = " + str(grp))					
				for t in range(len(grp)):
					s = grp[t].strip()
					proto = get_proto(s)
					if proto:
						grp.append(proto)
					else:
						table_key = 0
						error_count2 += 1
						error_strings2.append(s)
						continue
				grp[0:t+1]=[]
				print ("		group = " + str(tuple(grp)) )	
				table_list.append([odds,tuple(grp)])
					
			# process entries with dice rolls, example: '100,gold,2,10,0'
			# process simple entries, example: '75,boots of speed'
			elif entry.count(',') == 4 or entry.count(',') == 1:
				entry = entry.split(',')
				print ("		entry  = " + str(entry))					
				entry[0] = int(entry[0])
				s = entry[1].strip()
				proto = get_proto(s)
				if proto:
					entry[1] = proto
				else:
					table_key = 0
					error_count2 += 1
					error_strings2.append(s)
					continue
				if len(entry) == 2:
					table_list.append( [entry[0],entry[1]] )
				else:
					table_list.append( [entry[0],entry[1],int(entry[2]),int(entry[3]),int(entry[4])] )
			else:
				table_key = 0
				error_count2 += 1
				error_strings2.append(s)
	file_in.close()


#----------------------------------------------------------------
# Gets the proto number based on the english description
#----------------------------------------------------------------
def get_proto(s):
	proto = 0
	if s.islower() and s in descriptions.keys():
		proto = descriptions[s]
	elif s != "":
		proto = s
	return proto


#-----------------------------------------------------------
# Verify all keys that call inventory{2} are valid
# The keys can come from both inventory{} and inventory{2} 
#-----------------------------------------------------------

# Check calls from inv_source{}
def verify_inv_source():
	kys = inv_source.keys()
	kys.sort()
	for i in kys:
		for t in inv_source[i]:
			for e in t:
				if type (e[1]) is str:
					#print ( str(e[1]) )
					if inv_tables.get(e[1],"NO KEY") == "NO KEY":
						print ("*** INVALID TABLE REFERENCED IN inv_source{}: " + e[1])
				elif type (e[1]) is tuple:
					for eee in e[1]:
						if type(eee) is str:
							if inv_tables.get(eee,"NO KEY") == "NO KEY":
								print ("*** INVALID TABLE REFERENCED IN inv_source{}: " + eee)

# Check calls from inv_tables{}
def verify_inv_tables():
	kys = inv_tables.keys()
	kys.sort()
	for t in kys:
		for e in inv_tables[t]:
			if type (e[1]) is str:
				if inv_tables.get(e[1],"NO KEY") == "NO KEY":
					print ("*** INVALID TABLE REFERENCED in inv_tables{}: " + e[1])
			elif type (e[1]) is tuple:
				for eee in e[1]:
					if type(eee) is str:
						if inv_tables.get(eee,"NO KEY") == "NO KEY":
							print ("*** INVALID TABLE REFERENCED IN inv_tables{}: " + eee)


inv_source = {}
inv_tables = {}

error_count, error_strings = 0, []
error_count2, error_strings2 = 0, []

read_inv_tables()
read_inv_source()

# if os.getcwd().strip()[-4:] == "\\scr":
	# print ("Error Count reading inv_source = " + str(error_count) + "  " + str(error_strings) )
	# print ("Error Count reading inv_tables = " + str(error_count2) + "  " + str(error_strings2) )
	# verify_inv_source()
	# verify_inv_tables()


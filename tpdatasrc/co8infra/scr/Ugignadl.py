

def _boh_removeitem(bagnum, itemnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" Remove the item itemnum from the bag bagnum.  If it's not in there we get a (deliberate) crash. """
	allBagDict = readMes(bgfilename)
	contents = __boh_decode(allBagDict[bagnum])
	contents.remove(itemnum)
	allBagDict[bagnum] = contents
	writeMes(bgfilename, allBagDict)

def _boh_newbag(bagcontents=[], bagnum=0, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This function only takes keyword arguments.  bagnum of 0 will ask the function to use the lowest available, otherwise will use that number.  If it already exists the game will crash (deliberately, I can change that behaviour).  bgfilename is the filename for all the bags.  The contents should be a list (or tuple) of integers.   Also, if you want each BOH to start with something inside them here is where you can do it (say with a note on how to use them, about the charged things being broken and crafted stuff etc). """
	#  So we read in all the bags.
	allBagDict = readMes(bgfilename)
	#  Then insert the new bag.
	linenumbers = allBagDict.keys()
	if bagnum:
		if bagnum in allBagDict:
			raise 'BadbagnumError'
	else:
		bagnum = 1
		while bagnum in linenumbers:
			bagnum += 1
	allBagDict[bagnum] = bagcontents
	#  Then write it all back again.
	writeMes(bgfilename, allBagDict)
	return bagnum
    
def _boh_insertitem(bagnum, itemnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This function will insert the integer itemnum at the end of the list associated with bagnum in bgfilename. """
	allBagDict = readMes(bgfilename)
	contents = __boh_decode(allBagDict[bagnum])
	contents.append(itemnum)
	allBagDict[bagnum] = contents
	writeMes(bgfilename, allBagDict)

def _boh_getContents(bagnum, bgfilename='modules\\ToEE\\Bag_of_Holding.mes'):
	""" This can be called when opening the bag.  The bag filename is (I am assuming) fixed, but can be set by passing a keyword argument.  bagnum is the line number to use for that bag in the bgfilename. """
	allBagDict = readMes(bgfilename)
	#  Note:  If bagnum is not an integer we will have a CTD.  I can fix this if it is the case by coercing the datatype.
	contents = __boh_decode(allBagDict[bagnum])
	#  contents will be an ordered list of integers, ready to be turned into objects.
	return contents

def __boh_decode(bohcontents):
	""" bohcontents should just be a comma delimited series of integers.  We don't have any more support than that (CB: if we have more complex data types in the bag in future then this is the function to be changed.)   Sub-bags (i.e. BOH within BOH) could be implemented by decoding lists within lists.  However I'm not even sure that is in accordance with the rules. """
	# [0] is there to comply with the output of the readMes function.
	l = bohcontents[0].split(',')
	for i in range(len(l)):
		#  This should cause a crash, but I am testing with non-integers.  If you want to be hardcore then just remove the try--except.
		try:
			l[i] = int(l[i])
		except:
			print "WARNING: NON-INTEGER FOUND IN BAG OF HOLDING!"
	return l

def readTab(tabfile):
	""" Read the tab delimited tabfile into a dictionary indexed by line number. """
	tabFile = file(tabfile, 'r')
	tabDict = {}
	for line in tabFile.readlines():
		splitline = line.split('\t')
		tabDict[int(splitline[0])] = splitline[1:]
	tabFile.close()
	return tabDict

def wrap_writeTab(tabDict):
	""" Just a wrapper for the writeTab function.  Useful to make menu code pretty, and to uphold encapsulation *upholds encapsulation*. """
	#  Input an invalid filename and see if I care.
	tabfile = raw_input("What filename would you like for your *.tab file? ")
	return writetab(tabfile, tabDict)

def writeTab(tabfile, tabDict):
	""" Write the dictionary tabDict to tabfile.  Line endings may cause minor hassles.  Note that this will overwrite any file named tabfile in the process."""
	tabFile = file(tabTile, 'w')
	### REMOVED.  Just realised I should be focusing on *.mes files instead of *.tab.

def readMes(mesfile):
	""" Read the mesfile into a dictionary indexed by line number. """
	mesFile = file(mesfile, 'r')
	mesDict = {}
	for line in mesFile.readlines():
		# Remove whitespace.
		line = line.strip()
		# Ignore comment lines.
		if '//' in line:
			continue
		# Ignore empty lines.
		if not line:
			continue
		# Decode the line.  Just standard python string processing.
		line = line.split('}')[:-1]
		for i in range(len(line)):
			line[i] = line[i][1:]
		# Insert the line into the mesDict.
		mesDict[int(line[0])] = line[1:]
	mesFile.close()
	return mesDict

def wrap_outputline(dataDict):
	""" Wrapper for outputline. """
	line = int(raw_input("Which line would you like to check? "))
	print outputline(line, dataDict)
	print "(Note that some python characters are added to the output (line [, ], `, ', and perhaps some commas).  This will not affect the output when written to file.)"

def outputline(line, dataDict):
	""" Write entry corresponding to a particular line number to screen. """
	try:
		return dataDict[line]
	except:
		return "That line is not in the dictionary."

def wrap_writeMes(mesDict):
	""" Wrapper for writeMes. """
	mesfile = raw_input("What filename would you like to use? ")
	writeMes(mesfile, mesDict)

def writeMes(mesfile, mesDict):
	""" Write the dictionary mesDict as a mesfile.  This does not presever comments (although it could if desired).  Overwrites mesfile if it already exists."""
	mesFile = file(mesfile, 'w')
	linenumbers = mesDict.keys()
	linenumbers.sort()
	for linenumber in linenumbers:
		mesFile.write('{'+`linenumber`+'}{')
		for thing in mesDict[linenumber]:
			if thing == mesDict[linenumber][-1]:
				mesFile.write(`thing`)
			else:
				mesFile.write(`thing`+', ')
		mesFile.write('}\n')
	mesFile.close()

def wrap_editMes(mesDict):
	""" Wrapper for editMes. """
	editMes(mesDict)

def editMes(mesDict):
	""" Edit the mesDict.  Can add a new entry or edit an existing one.  Note that if you enter a line number which is not in the dict the function will still work."""
	linenumbers = mesDict.keys()
	newnum = int(raw_input("Please enter the desired line number (0 for lowest available) >> "))
	if newnum:
		if newnum in mesDict:
			print "The current entry for this line reads:"
			print mesDict[newnum]
		else:
			print "Line number ", newnum, "is available."
	else:
		# Set line to lowest available.
		newnum = 1
		while newnum in linenumbers:
			newnum += 1
		print "The lowest available line number is", newnum, "."
	print "The entry will be stored as a comma delimited list."
	print "For example:", "{", newnum, "}{ <entry1>, <entry2>, ... ,<entryn>}"
	entrynum = int(raw_input("How many entries would you like for line "+`newnum`+"? "))
	entry = []
	for i in range(entrynum):
		entry.append(raw_input("Please input entry "+`i+1`+": "))
	print "I will now insert the following entry into the current dictionary:"
	print "line:", newnum,", data:",entry
	if raw_input("Is this OK? (y/n) ") == 'y':        
		#  Note to readers:  I could check with the protos.tab, and other mes files, for validity?  Would this be useful?
		mesDict[newnum] = entry
		print "Done."
	else:
		print "Fine, fine.  No entry for you."
    
def menu():
	print
	print "Please choose one of the following options:"
	print
	print "1. Write the current dictionary to a *.tab file. (REMOVED)"
	print "2. Output a line from the data dictionary."
	print "3. Write the current dictionary to a *.mes file."
	print "4. Add a new entry to the current dictionary."
	print "5. Quit."
	try:
		option = int(raw_input(">> "))
		assert 1 <= option <= 5
		if option == 5:
			option = 0
		return option
	except:
		print "Please choose a valid option."
		#  Yeah maximum recursion limit will break this.  Witness how much I care :D.
		return menu()

if __name__ == '__main__':
	print "Welcome to Ugignadl's trivial MES/TAB manipulator."
	print
	print "PLEASE BACK UP YOUR *.mes AND *.tab FILES BEFORE USING THIS PROGRAM!"
	print
	#  This can be read from user input if desired.
	# Or just edited in here manually.  For those
	# UNIX peeps, this could also be read from stdin.
	# (Although who runs TOEE on *nix?)
	print "Reading input file..."
	datafile = 'bonus.mes'
	dataDict = readMes(datafile)
	print "Input file", datafile,"read."
	#  Now we just ask the user what they want to do.
	# The possibilities are MUCH greater than what
	# I have here right now.  In fact, regardless of
	# what it is, I would ask me (does that even make
	# sense?) to implement it, as it is most likely
	# not only possible but easy.
	choices = {1:wrap_writeTab, 2:wrap_outputline, 3:wrap_writeMes, 4:wrap_editMes}
	option = -1
	while option:
		option = menu()
		if option:
			choices[option](dataDict)
	print "Goodbye!"


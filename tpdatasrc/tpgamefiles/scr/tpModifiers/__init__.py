import tpdp

print "Attempting import of tpModifiers"
flist = tpdp.GetModifierFileList()

print "Found  " + str(len(flist)) + " files."

for p in range(0, len(flist)):
	#print str(flist[p])
	try:
		__import__('tpModifiers.' + flist[p])
	except Exception as e:
		print "Unexpected error: ", str(e), "Shutting down"
		exit()


print "tpModifiers import finished"
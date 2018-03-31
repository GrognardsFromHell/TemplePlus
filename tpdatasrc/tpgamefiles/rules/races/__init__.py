print "Initing races module"
import race_defs
import importlib


print "Attempting import of races"
flist = race_defs.GetRaceFileList()

print "Found  " + str(len(flist)) + " files."

for p in range(0, len(flist)):
	print str(flist[p])
	try:
		#m = __import__('races.' + flist[p])
		m = importlib.import_module('races.' + flist[p])
		#print dir(m)
		m.RegisterRace()
	except Exception as e:
		print "Error: ", str(e)
		exit()


print "races import finished"
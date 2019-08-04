##Co8 persistent data scheme - Spellslinger
##Requires dllfix16 or higher.
import types
print "Initializing Co8 Persistent Data module"


def buildPath(fname):
    print "modules\\ToEE\\save\\" + fname + ".co8"
    """Helper function to construct the save path"""
    return "modules\\ToEE\\save\\" + fname + ".co8" # modified for TemplePlus


def isNone(obj):
    if type(obj) == types.NoneType:
        return 1
    return 0




class Co8PersistentData(object):
    """Provides the static data for Co8 saves.
        Do NOT use the load and save methods from your code!"""
    __dataDict = dict()
#    __registeredKeys

    def setData(key, value):
        r"""Add the data entry with that key(string only).
            
            Do not use | in the key name.
            Do not use \n in directly passed strings.
            If you need to save strings with \n pack them into a list/tuple."""
        #only string keys are accepted
        if ( type(key) != str ): return None
        
        Co8PersistentData.__dataDict[key] = value
    setData = staticmethod(setData)


    def getData(key):
        """Return the data entry with that key(string only).
        If the key is not in the data, return None.
        
        After loading the data from file:
        If the data inserted was a string or number, return a string
        representation.
        If the data inserted was a list, tuple or dict, return the
        original type."""
        #only string keys are accepted
        if ( type(key) != str ): return None
        
        try:
            return Co8PersistentData.__dataDict[key]
        except KeyError:
            return None
    getData = staticmethod(getData)


    def removeData(key):
        """Remove the data entry with that key(string only)."""
        #only string keys are accepted
        if ( type(key) != str ): return None
        
        try:
            del Co8PersistentData.__dataDict[key]
        except KeyError:
            pass            
    removeData = staticmethod(removeData)
        

    def save(savename):
        print "Attempting save...\n"
        """Save the data to file.
            DO NOT USE FROM CLIENT CODE!"""
        try:
            print "Attempting to open file...\n"
            print "savename: " + savename
            print str(savename)+"\n"
            out = open(buildPath(savename), "w")
        except IOError:
            return
        
        #write all dict entries to file, format: "key|value\n"
        for key, value in Co8PersistentData.__dataDict.items():
            out.write(key + "|")
            out.write(str(value) + "\n")
        out.close()
    save = staticmethod(save)


    def load(savename):
        """Load the data from file.
            DO NOT USE FROM CLIENT CODE!"""
        Co8PersistentData.__dataDict.clear()
        try:
            inFile = open(buildPath(savename), "r")
        except IOError:
            return

        for line in inFile:
            separatorPos = line.index("|")
            #read up to separator
            key = line[:separatorPos]
            #read from separator to -1 to ignore \n
            value = line[separatorPos + 1:-1]
            #restore lists, tuples, dicts
            if ( value[0] == "(" or value[0] == "[" or value[0] == "{" ):
                exec("value = " + value)
            Co8PersistentData.__dataDict[key] = value
            
        inFile.close()
    load = staticmethod(load)

    

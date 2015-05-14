
#use 4 space indentation!

# save hook (default - Co8 hook; will silently fail for vanilla; feel free to change for your own custom mod!)
def save(filename):
    try:
        import _co8init
        print "imported Co8Init inside templeplus package"
        _co8init.save(filename)
        print "Save hook successful"
    except:
        print "Save hook failed\n"

# save hook (default - Co8 hook; will silently fail for vanilla; feel free to change for your own custom mod!)
def load(filename):
    try:
        import _co8init
        _co8init.load(filename)
    except:
        print "Load hook failed\n"

#use 4 space indentation!

def save(filename):
    return

def load(filename):
    return

# Co8 python save hook - executes after the archive is done
# save hook (default - Co8 hook; will silently fail for vanilla; feel free to change for your own custom mod!)
def post_save(filename):
    try:
        import _co8init
        print "imported Co8Init inside templeplus package"
        _co8init.save(filename)
        print "Co8 Save hook successful"
    except:
        print "Co8 Save hook failed\n"

# save hook (default - Co8 hook; will silently fail for vanilla; feel free to change for your own custom mod!)
def post_load(filename):
    try:
        import _co8init
        _co8init.load(filename)
    except:
        print "Co8 Load hook failed\n"
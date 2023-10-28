from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    print "CheckPrereq Hook for GSF Invocation"
    if not char_editor.has_feat("Spell Focus (Invocation)"):
        print "Does not have feat requirements"
        return 0
    return 1

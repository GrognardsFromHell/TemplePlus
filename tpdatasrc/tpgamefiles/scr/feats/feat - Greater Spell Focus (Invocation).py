from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if not char_editor.has_feat("Spell Focus (Invocation)"):
        return 0
    return 1

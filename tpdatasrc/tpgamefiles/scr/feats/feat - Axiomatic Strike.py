from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Axiomatic Strike requires Ki Strike (lawful) which would be Monk 7
    #Ki Strike is gained at Monk level 4 and initionally grants Magic DAP
    #Ki Strike actually never grants Axiomatic DAP in the game
    #Due to this I decided to set the feat to prereq Monk 7
    #Which is handled by the engine
    #Stunning Fist Check
    if not char_editor.has_feat(feat_stunning_fist):
        return 0
    return 1

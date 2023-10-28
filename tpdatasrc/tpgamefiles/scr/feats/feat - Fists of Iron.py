from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #BAB +2 checked by engine
    #Stunning Fist Check
    if not char_editor.has_feat(feat_stunning_fist):
        return 0
    # Improved Unarmed Strike Check
    if not char_editor.has_feat(feat_improved_unarmed_strike):
        return 0
    return 1

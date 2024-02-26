from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Prerequisite of Base Attack Bonus of 3 is handled by Engine
    if not char_editor.has_feat(feat_shield_proficiency):
        return 0
    return 1

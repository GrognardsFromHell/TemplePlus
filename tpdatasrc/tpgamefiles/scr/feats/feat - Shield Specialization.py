from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.has_feat(feat_shield_proficiency):
        return 1
    return 0

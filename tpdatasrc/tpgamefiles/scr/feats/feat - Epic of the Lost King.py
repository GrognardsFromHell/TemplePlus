from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.has_feat(feat_bardic_music):
        return 1
    return 0

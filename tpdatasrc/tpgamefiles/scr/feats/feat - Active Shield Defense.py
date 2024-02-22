from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if not char_editor.has_feat(feat_shield_proficiency):
        return 0
    elif not char_editor.has_feat("Shield Specialization"):
        return 0
    return 1

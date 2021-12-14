from toee import *
import char_editor
from heritage_feat_utils import hasDifferentHeritageFeat

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.stat_level_get(stat_alignment) & ALIGNMENT_LAWFUL:
        return 0
    #Check if character already has any heritage feats
    if hasDifferentHeritageFeat(char_editor, "Fey Heritage"):
        return 0
    return 1

from toee import *
import char_editor
from heritage_feat_utils import hasDifferentHeritageFeat

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.stat_level_get(stat_level_sorcerer) < 1:
        return 0
    #Check if character already has any heritage feats
    if hasDifferentHeritageFeat(char_editor, "Draconic Heritage"):
        return 0
    return 1

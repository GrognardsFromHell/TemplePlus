from toee import *
import char_editor
from heritage_feat_utils import hasDifferentHeritageFeat

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Sorcerer Level 1 is handled by the engine
    #Check if character already has any heritage feats
    if hasDifferentHeritageFeat(char_editor, "Draconic Heritage"):
        return 0
    return 1

from toee import *
import char_editor
from heritage_feat_utils import getDraconicHeritageColourString

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Sorcerer Level 1 is handled by the engine
    hasDraconicHeritageFeat = False
    for heritage in range(heritage_draconic_black, heritage_draconic_white + 1):
        colourString = getDraconicHeritageColourString(heritage)
        if char_editor.has_feat("Draconic Heritage {}".format(colourString)):
            hasDraconicHeritageFeat = True
            break
    if not hasDraconicHeritageFeat:
        return 0
    return 1

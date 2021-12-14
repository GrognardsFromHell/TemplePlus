from templeplus.pymod import PythonModifier
from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.has_feat("Dragon Disciple Breath Weapon"):
        return 1
    elif char_editor.has_feat("Dragon Shaman Breath Weapon"):
        return 1
    return 0

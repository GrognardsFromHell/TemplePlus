from templeplus.pymod import PythonModifier
from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Check if character has a limited usage Breath Weapon
    #At the moment only Dragon Diciple has one:
    if not char_editor.has_feat("Dragon Disciple Breath Weapon"):
        return 0
    return 1

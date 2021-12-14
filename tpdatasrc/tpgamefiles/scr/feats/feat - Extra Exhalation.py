from templeplus.pymod import PythonModifier
from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    #Check if character has a limited usage Breath Weapon
    if attachee.d20_query("PQ_Has_Limited_Breath_Weapon"):
        return 1
    return 0

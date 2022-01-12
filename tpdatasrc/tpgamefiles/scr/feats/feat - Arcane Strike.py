from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    # BAB +4 or higher is checked by the engine
    if attachee.arcane_spell_level_can_cast() >= 3:
        return 1
    return 0

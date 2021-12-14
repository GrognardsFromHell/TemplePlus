from toee import *
import char_editor

def CheckPrereq(attachee, classLevelled, abilityScoreRaised):
    if char_editor.stat_level_get(stat_level) < 5:
        return 0
    if char_editor.stat_level_get(stat_alignment) & ALIGNMENT_LAWFUL:
        return 0
    if not char_editor.has_feat("Fey Heritage"):
        return 0
    return 1

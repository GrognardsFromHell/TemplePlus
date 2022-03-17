from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *

#This file contains all helper conditions to remove special conditions
#like fatigue or nauseated that are not already handled by a vanilla spell

### WIP ###

def removeSelf(attachee, args, evt_obj):
    #particlesString = args.get_cond_name()
    #game.particles(particlesString, attachee)
    args.remove_spell_mod()
    args.remove_spell()
    return 0

class RemoveConditionModifier(PythonModifier):
    # RemoveConditionModifier have 3 arguments:
    # 0: spell_id, 1: duration, 2: empty
    def __init__(self, name):
        PythonModifier.__init__(self, name, 3, False)
        self.AddHook(ET_OnConditionAdd, EK_NONE, removeSelf, ())
        self.AddSpellDispelCheckStandard()
        self.AddSpellTeleportPrepareStandard()
        self.AddSpellTeleportReconnectStandard()
        self.AddSpellCountdownStandardHook()

removeExhaustion = RemoveConditionModifier("sp-Remove Exhaustion")
removeFatigue = RemoveConditionModifier("sp-Remove Fatigue")
removeNauseated = RemoveConditionModifier("sp-Remove Nauseated")
removeShaken = RemoveConditionModifier("sp-Remove Shaken")
removeSickened = RemoveConditionModifier("sp-Remove Sickened")

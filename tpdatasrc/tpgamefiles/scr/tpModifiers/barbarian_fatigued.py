from templeplus.pymod import PythonModifier
from toee import *
import tpdp

def additionalRemoval(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Remove Exhaustion") or evt_obj.is_modifier("sp-Remove Fatigue"):
        args.condition_remove()
    return 0

extendBarbarianFatigued = PythonModifier()
extendBarbarianFatigued.ExtendExisting("Barbarian_Fatigued")
extendBarbarianFatigued.AddHook(ET_OnConditionAddPre, EK_NONE, additionalRemoval, ())

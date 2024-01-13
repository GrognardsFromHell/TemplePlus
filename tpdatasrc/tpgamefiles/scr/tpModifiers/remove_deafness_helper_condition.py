from templeplus.pymod import PythonModifier
from toee import *
import tpdp

##### sp-Deafness Remove Helper Condition#####

def removeDeafness(attachee, args, evt_obj):
    spellId = 0
    duration = 0
    attachee.condition_add_with_args('sp-Remove Deafness', spellId, duration, 0)
    return 0

def conditionTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

remDeafHelper = PythonModifier("Remove Deafness Helper Condition", 2) #duration, empty
remDeafHelper.AddHook(ET_OnConditionRemove, EK_NONE, removeDeafness, ())
remDeafHelper.AddHook(ET_OnBeginRound, EK_NONE, conditionTickdown, ())
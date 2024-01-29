from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Dismiss Extender"

def RemoveDismissCondition(attachee, args, evt_obj):
    if args.get_arg(0) == evt_obj.data1:
        args.condition_remove()
    return 0

powerAttackExtender = PythonModifier()
powerAttackExtender.ExtendExisting("Dismiss")
powerAttackExtender.AddHook(ET_OnD20PythonSignal, "Remove Dismiss Condition", RemoveDismissCondition, ())
from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Dead extender"

def addRecentlyDeceased(attachee, args, evt_obj):
    duration = 1
    attachee.condition_add_with_args("Recently Deceased", duration, 0)
    return 0

modExtender = PythonModifier()
modExtender.ExtendExisting("Dead")
modExtender.AddHook(ET_OnConditionAdd, EK_NONE, addRecentlyDeceased, ())

def tickDown(attachee, args, evt_obj):
    duration = args.get_arg(0)
    duration -= evt_obj.data1
    args.set_arg(0, duration)
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def queryRecentlyDeceased(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def tooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    evt_obj.append(conditionName)
    return 0

def rezSignal(attachee, args, evt_obj):
    args.condition_remove()
    return 0

recentlyDeceasedCond = PythonModifier("Recently Deceased", 2) #duration, empty
recentlyDeceasedCond.AddHook(ET_OnBeginRound, EK_NONE, tickDown, ())
recentlyDeceasedCond.AddHook(ET_OnD20PythonQuery, "PQ_Recently_Deceased", queryRecentlyDeceased, ())
recentlyDeceasedCond.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())
recentlyDeceasedCond.AddHook(ET_OnD20Signal, EK_S_Resurrection, rezSignal, ())

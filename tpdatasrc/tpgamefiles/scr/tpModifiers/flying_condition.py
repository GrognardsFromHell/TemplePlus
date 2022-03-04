from templeplus.pymod import PythonModifier
from __main__ import game
from toee import *
import tpdp

def floatImmunity(attachee, immunityEffect, immunityTag):
    attachee.float_text_line("Immune due to flying", tf_red)
    game.create_history_freeform("{} is immune to ~{}~[{}] effects\n\n".format(attachee.description, immunityEffect, immunityTag))

def preventConditions(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Grease Hit"):
        evt_obj.return_val = 0
        floatImmunity(attachee, "Grease", "TAG_SPELLS_GREASE")
    elif evt_obj.is_modifier("sp-Entangle On"):
        evt_obj.return_val = 0
        floatImmunity(attachee, "Entangle", "TAG_SPELLS_ENTANGLE")
    return 0

def preventAoO(attachee, args, evt_obj):
    attachee.float_text_line("Flying!")
    evt_obj.return_val = 0
    return 0

def queryIsFlying(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def tooltipFlying(attachee, args, evt_obj):
    evt_obj.append("Flying!")
    return 0

def signalStopFlying(attachee, args, evt_obj):
    args.condition_remove()
    return 0

flyingCondition = PythonModifier("Flying Condition", 3) #empty, empty, empty
flyingCondition.AddHook(ET_OnConditionAddPre, EK_NONE, preventConditions, ())
flyingCondition.AddHook(ET_OnD20Query, EK_Q_AOOIncurs, preventAoO,())
flyingCondition.AddHook(ET_OnGetTooltip, EK_NONE, tooltipFlying, ())
flyingCondition.AddHook(ET_OnD20PythonQuery, "PQ_Is_Flying", queryIsFlying, ())
flyingCondition.AddHook(ET_OnD20PythonSignal, "PS_Flying_End", signalStopFlying, ())

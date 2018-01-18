from toee import *
import tpactions
import tpdp


def GetActionName():
    return "Arrow of Death"


def GetActionDefinitionFlags():
    return D20ADF_None


def GetTargetingClassification():
    return D20TC_Target0


def GetActionCostType():
    return D20ACT_NULL


def AddToSequence(d20action, action_seq, tb_status):
    print " aded death attack to seq"
    action_seq.add_action(d20action)
    return AEC_OK
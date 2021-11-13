from toee import *
import tpactions

def GetActionName():
    return "Marshal Learn Minor Aura"

def GetActionDefinitionFlags():
    return D20ADF_None
    
def GetTargetingClassification():
    return D20TC_Target0

def GetActionCostType():
    return D20ACT_NULL

def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK
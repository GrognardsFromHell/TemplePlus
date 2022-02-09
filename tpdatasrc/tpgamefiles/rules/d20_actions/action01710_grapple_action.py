from toee import *
import tpactions

def GetActionName():
    return "Grapple Action"

def GetActionDefinitionFlags():
    return D20ADF_TriggersAoO | D20ADF_TriggersCombat | D20ADF_Breaks_Concentration 
    
def GetTargetingClassification():
    return D20TC_SingleExcSelf

def GetActionCostType():
    return D20ACT_Standard_Action

def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK
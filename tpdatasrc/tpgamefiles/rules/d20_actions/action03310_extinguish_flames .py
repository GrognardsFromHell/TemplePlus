from toee import *
import tpactions

def GetActionName():
    return "Extinguish Flames"

def GetActionDefinitionFlags():
    return D20ADF_TriggersAoO | D20ADF_Breaks_Concentration
    
def GetTargetingClassification():
    return D20TC_Target0

def GetActionCostType():
    return D20ACT_Full_Round_Action

def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK
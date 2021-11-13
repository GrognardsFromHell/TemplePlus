from toee import *
import tpactions

def GetActionName():
    return "Activate Marshal Minor Aura"

def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting

def GetTargetingClassification():
    return D20TC_CastSpell

def GetActionCostType():
    return D20ACT_Swift_Action

def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK
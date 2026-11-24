from toee import *
import tpactions

def GetActionName():
    return "Dragon Disciple Breath Weapon"

def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting | D20ADF_Breaks_Concentration | D20ADF_TriggersCombat

def GetTargetingClassification():
    return D20TC_CastSpell

def GetActionCostType():
    return D20ACT_Standard_Action

def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK

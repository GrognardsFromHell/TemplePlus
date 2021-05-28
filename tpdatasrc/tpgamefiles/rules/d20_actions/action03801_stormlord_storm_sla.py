from toee import *
import tpactions


def GetActionName():
    return "Stormlord Storm SLA"


def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting | D20ADF_Breaks_Concentration | D20ADF_QueryForAoO


def GetTargetingClassification():
    return D20TC_CastSpell


def GetActionCostType():
    return D20ACT_Full_Round_Action


def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK

#def ModifyPicker( picker_args ):
#    picker_args.set_mode_target_flag(tpactions.ModeTarget.Area) #PickOrigin
#    return 1

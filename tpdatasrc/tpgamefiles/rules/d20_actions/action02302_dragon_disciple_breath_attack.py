from toee import *
import tpactions
import tpdp


def GetActionName():
    return "Dragon Disciple Breath Attack"


def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting | D20ADF_QueryForAoO


def GetTargetingClassification():
    return D20TC_CastSpell


def GetActionCostType():
    return D20ACT_Standard_Action


def AddToSequence(d20action, action_seq, tb_status):
    print "Dragon Disciple Breath Weapon Add to Sequence"
    if d20action.performer.d20_query(Q_Prone):
        d20aGetup = d20action
        d20aGetup.action_type = tpdp.D20ActionType.StandUp
        action_seq.add_action(d20aGetup)
    action_seq.add_action(d20action)
    return AEC_OK

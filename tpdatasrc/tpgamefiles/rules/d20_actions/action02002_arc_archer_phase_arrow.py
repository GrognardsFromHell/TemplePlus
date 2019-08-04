from toee import *
import tpactions


def GetActionName():
    return "Phase Arrow"


def GetActionDefinitionFlags():
    return D20ADF_TargetSingleExcSelf | D20ADF_Breaks_Concentration | D20ADF_TriggersCombat | D20ADF_UseCursorForPicking


def GetTargetingClassification():
    return D20TC_SingleExcSelf


def GetActionCostType():
    return D20ACT_Standard_Action


def AddToSequence(d20action, action_seq, tb_status):

    if d20action.performer.d20_query(Q_Prone):
        d20aGetup = d20action
        d20aGetup.action_type = D20A_STAND_UP
        action_seq.add_action(d20aGetup)

    action_seq.add_action(d20action)
    return AEC_OK

def ProjectileHit(d20action, proj, obj2):
    print "Seeker Arrow: Projectile Hit"

    game.create_history_from_id(d20action.roll_id_1)
    game.create_history_from_id(d20action.roll_id_2)
    game.create_history_from_id(d20action.roll_id_0)
    print "Projectile Hit Target: " + str(d20action.target)
    d20action.target.deal_attack_damage(d20action.performer, d20action.data1, d20action.flags, d20action.action_type)
    d20action.performer.apply_projectile_hit_particles(proj, d20action.flags)


    return 1
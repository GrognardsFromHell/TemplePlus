from toee import *
import tpactions
import tpdp


def GetActionName():
    return "Hail of Arrows"


def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting | D20ADF_Breaks_Concentration | D20ADF_TriggersCombat


def GetTargetingClassification():
    return D20TC_CastSpell


def GetActionCostType():
    return D20ACT_Standard_Action


def AddToSequence(d20action, action_seq, tb_status):
    print "Hail of Arrows AddToSequence"
    if d20action.performer.d20_query(Q_Prone):
        d20aGetup = d20action
        d20aGetup.action_type = tpdp.D20ActionType.StandUp
        action_seq.add_action(d20aGetup)

    action_seq.add_action(d20action)

    # add D20A_STANDARD_RANGED_ATTACK actions for each target from the spell
    for p in range(0, 32):
        tgt = action_seq.spell_packet.get_target(p)
        if tgt == OBJ_HANDLE_NULL:
            break
        print "Adding standard ranged attack"

        d20aAtk = d20action
        d20aAtk.performer = d20action.performer
        d20aAtk.action_type = tpdp.D20ActionType.StandardRangedAttack
        d20aAtk.data1 = 1
        d20aAtk.target = tgt
        d20aAtk.flags = D20CAF_FREE_ACTION
        action_seq.add_action(d20aAtk)

    return AEC_OK

def ProjectileHit(d20action, proj, obj2):
    print "Hail of Arrows: Projectile Hit"

    game.create_history_from_id(d20action.roll_id_1)
    game.create_history_from_id(d20action.roll_id_2)
    game.create_history_from_id(d20action.roll_id_0)
    print "Projectile Hit Target: " + str(d20action.target)
    d20action.target.deal_attack_damage(d20action.performer, d20action.data1, d20action.flags, d20action.action_type)
    d20action.performer.apply_projectile_hit_particles(proj, d20action.flags)


    return 1
from toee import *
import tpactions


def GetActionName():
    return "Warlock Eldritch Chain"


def GetActionDefinitionFlags():
    return D20ADF_MagicEffectTargeting | D20ADF_Breaks_Concentration | D20ADF_QueryForAoO


def GetTargetingClassification():
    return D20TC_CastSpell


def GetActionCostType():
    return D20ACT_Standard_Action


def AddToSequence(d20action, action_seq, tb_status):
    action_seq.add_action(d20action)
    return AEC_OK

def ProjectileHit(d20action, proj, obj2):
    d20action.performer.apply_projectile_hit_particles(proj, d20action.flags)
    tpactions.trigger_spell_projectile(d20action.spell_id, proj)
    return 1
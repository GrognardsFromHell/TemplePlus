from toee import *
import tpdp
from templeplus.pymod import BasicPyMod

print "Registering Dazzled"

def dazzledConditionAddPreActions(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Heal")
    or evt_obj.is_modifier("sp-Remove Dazzled")):
        args.condition_remove()
    return 0

def dazzledConditionAddActions(attachee, args, evt_obj):
    #Only creatures with sight are affected
    if attachee.is_category_type(mc_type_ooze):
        spellMesId = 32000 #ID 32000 = Target is immune!
        attachee.float_mesfile_line("mes\\spell.mes", spellMesId)
        args.condition_remove()
    elif attachee.d20_query(Q_Critter_Is_Blinded):
        attachee.float_text_line("Unaffected to due Blindness")
        args.condition_remove()
    else:
        attachee.float_text_line("Dazzled", tf_red)
        game.create_history_freeform("{} is ~Dazzled~[TAG_Dazzled]\n\n".format(attachee.description))
        particlesId = game.particles("sp-Dazzled", attachee)
        args.set_arg(2, particlesId)
    return 0

def dazzledTickdown(attachee, args, evt_obj):
    persistentFlag = args.get_arg(1)
    if not persistentFlag:
        duration = args.get_arg(0)
        duration -= evt_obj.data1
        args.set_arg(0, duration)
        if args.get_arg(0) < 0:
            args.condition_remove()
    return 0

def queryHasCondition(attachee, args, evt_obj):
    queryConditionRef = evt_obj.data1
    conditionName = args.get_cond_name()
    DazzledCondRef = tpdp.get_condition_ref(conditionName)
    if queryConditionRef == DazzledCondRef:
        evt_obj.return_val = 1
    return 0

def signalUpdateDuration(attachee, args, evt_obj):
    duration = evt_obj.data1
    persistentFlag = evt_obj.data2
    args.set_arg(0, duration)
    args.set_arg(1, persistentFlag)
    return 0

def dazzledRemoveCondition(attachee, args, evt_obj):
    particlesId = args.get_arg(2)
    if particlesId:
        game.particles_end(particlesId)
    return 0

def getDurationLabel(args):
    duration = args.get_arg(0)
    persistentFlag = args.get_arg(1)
    if persistentFlag:
        return "persistent"
    elif duration == 1:
        return "1 round"
    return "{} rounds".format(duration)

def dazzledTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    durationLabel = getDurationLabel(args)
    evt_obj.append("{} ({})".format(conditionName, durationLabel))
    return 0

def dazzledEffectTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    conditionKey = conditionName.upper().replace(" ", "_")
    durationLabel = getDurationLabel(args)
    evt_obj.append(tpdp.hash(conditionKey), -2, " ({})".format(durationLabel))
    return 0

def applyPenalty(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    conditionTag = "TAG_{}".format(conditionName.upper().replace(" ", "_"))
    bonus = -1 # Dazzled is a -1 penalty to attack rolls and to search and spot checks
    bonusType = bonus_type_dazzled
    bonusHelpTag = game.get_mesline("mes\\bonus_description.mes", bonusType)
    evt_obj.bonus_list.add(bonus, bonusType, "{} : ~{}~[{}]".format(bonusHelpTag, conditionName, conditionTag))
    return 0


dazzledCondition = BasicPyMod("Dazzled", 4, False) #duration, persistentFlag, particlesId, empty
dazzledCondition.AddHook(ET_OnConditionAddPre, EK_NONE, dazzledConditionAddPreActions, ())
dazzledCondition.AddHook(ET_OnConditionAdd, EK_NONE, dazzledConditionAddActions, ())
dazzledCondition.AddHook(ET_OnBeginRound, EK_NONE, dazzledTickdown, ())
dazzledCondition.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Condition, queryHasCondition, ())
dazzledCondition.AddHook(ET_OnD20PythonSignal, "PS_Dazzled_Update_Duration", signalUpdateDuration, ())
dazzledCondition.AddHook(ET_OnConditionRemove, EK_NONE, dazzledRemoveCondition, ())
dazzledCondition.AddHook(ET_OnGetTooltip, EK_NONE, dazzledTooltip, ())
dazzledCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, dazzledEffectTooltip, ())
dazzledCondition.AddHook(ET_OnGetSkillLevel, EK_SKILL_SEARCH, applyPenalty, ())
dazzledCondition.AddHook(ET_OnGetSkillLevel, EK_SKILL_SPOT, applyPenalty, ())
dazzledCondition.AddHook(ET_OnToHitBonus2, EK_NONE, applyPenalty, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import isLivingCreature

print "Registering Nauseated"

def nauseatedConditionAddPreActions(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Heal")
    or evt_obj.is_modifier("sp-Remove Nauseated")):
        args.condition_remove()
    return 0

def nauseatedConditionAddActions(attachee, args, evt_obj):
    #Only living creatures are affected by nausea
    if isLivingCreature(attachee):
        attachee.float_text_line("Nauseated", tf_red)
        game.create_history_freeform("{} is ~nauseated~[TAG_NAUSEATED]\n\n".format(attachee.description))
        particlesId = game.particles("sp-Disease-Filth Fever", attachee) #sp-Poison
        args.set_arg(2, particlesId)
        if attachee.d20_query(Q_Critter_Is_Concentrating) == 1: #Nauseated breaks concentration
            attachee.d20_send_signal(S_Remove_Concentration)
    else:
        spellMesId = 32000 #ID 32000 = Target is immune!
        attachee.float_mesfile_line("mes\\spell.mes", spellMesId)
        args.condition_remove()
    return 0

def nauseatedTickdown(attachee, args, evt_obj):
    persistentFlag = args.get_arg(1)
    if not persistentFlag:
        duration = args.get_arg(0)
        duration -= evt_obj.data1
        args.set_arg(0, duration)
        if args.get_arg(0) < 0:
            args.condition_remove()
    return 0

#Nauseated condition limits to a single Move Action only
def setTurnBasedStatusInit(attachee, args, evt_obj):
    if evt_obj.tb_status.hourglass_state > 1:
        evt_obj.tb_status.hourglass_state = 1
    return 0

#Can't AoO under Nauseated condition
def queryAnswerFalse(attachee, args, evt_obj):
    evt_obj.return_val = 0
    return 0

def queryAnswerTrue(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def queryHasCondition(attachee, args, evt_obj):
    queryConditionRef = evt_obj.data1
    conditionName = args.get_cond_name()
    nauseatedCondRef = tpdp.get_condition_ref(conditionName)
    if queryConditionRef == nauseatedCondRef:
        evt_obj.return_val = 1
    return 0

def signalUpdateDuration(attachee, args, evt_obj):
    duration = evt_obj.data1
    persistentFlag = evt_obj.data2
    args.set_arg(0, duration)
    args.set_arg(1, persistentFlag)
    return 0

def nauseatedRemoveCondition(attachee, args, evt_obj):
    if isLivingCreature(attachee) and attachee.stat_level_get(stat_hp_current) > -10:
        attachee.float_text_line("No longer nauseated")
        game.create_history_freeform("{} is no longer ~nauseated~[TAG_NAUSEATED]\n\n".format(attachee.description))
    particlesId = args.get_arg(2)
    if particlesId:
        game.particles_end(particlesId)
    return 0

def getDurationLabel(duration):
    if duration == 1:
        return "1 round"
    return "{} rounds".format(duration)

def nauseatedTooltip(attachee, args, evt_obj):
    duration = args.get_arg(0)
    persistentFlag = args.get_arg(1)
    conditionName = args.get_cond_name()
    durationLabel = "persistent" if persistentFlag else getDurationLabel(duration)
    evt_obj.append("{} ({})".format(conditionName, durationLabel))
    return 0

def nauseatedEffectTooltip(attachee, args, evt_obj):
    duration = args.get_arg(0)
    persistentFlag = args.get_arg(1)
    conditionName = args.get_cond_name()
    conditionKey = conditionName.upper().replace(" ", "_")
    durationLabel = "persistent" if persistentFlag else getDurationLabel(duration)
    evt_obj.append(tpdp.hash(conditionKey), -2, " ({})".format(durationLabel))
    return 0

nauseatedCondition = PythonModifier("Nauseated", 4) #duration, persistentFlag, particlesId, empty
nauseatedCondition.AddHook(ET_OnConditionAddPre, EK_NONE, nauseatedConditionAddPreActions, ())
nauseatedCondition.AddHook(ET_OnConditionAdd, EK_NONE, nauseatedConditionAddActions, ())
nauseatedCondition.AddHook(ET_OnTurnBasedStatusInit, EK_NONE, setTurnBasedStatusInit, ())
nauseatedCondition.AddHook(ET_OnBeginRound, EK_NONE, nauseatedTickdown, ())
nauseatedCondition.AddHook(ET_OnD20Query, EK_Q_CannotCast, queryAnswerTrue, ())
nauseatedCondition.AddHook(ET_OnD20Query, EK_Q_AOOPossible, queryAnswerFalse, ())
nauseatedCondition.AddHook(ET_OnD20Query, EK_Q_Critter_Has_Condition, queryHasCondition, ())
nauseatedCondition.AddHook(ET_OnD20PythonSignal, "PS_Nauseated_Update_Duration", signalUpdateDuration, ())
nauseatedCondition.AddHook(ET_OnConditionRemove, EK_NONE, nauseatedRemoveCondition, ())
nauseatedCondition.AddHook(ET_OnGetTooltip, EK_NONE, nauseatedTooltip, ())
nauseatedCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, nauseatedEffectTooltip, ())

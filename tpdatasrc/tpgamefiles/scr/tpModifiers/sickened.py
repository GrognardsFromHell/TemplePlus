from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import isLivingCreature

print "Registering Sickened"

def checkConditionalRemoval(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Heal")
    or evt_obj.is_modifier("sp-Remove Sickened")
    or evt_obj.is_modifier("Sickened")):
        args.condition_remove()
    return 0

def checkImmunity(attachee, args, evt_obj):
    if isLivingCreature(attachee):
        spellMesId = 20026 #ID 20026 = Sickened!
        attachee.float_mesfile_line("mes\\spell.mes", spellMesId, tf_red)
        game.create_history_freeform("{} is ~sickened~[TAG_SICKENED]\n\n".format(attachee.description))
        particlesId = game.particles("sp-Disease-Filth Fever", attachee)
        args.set_arg(1, particlesId)
    else:
        spellMesId = 32000 #ID 32000 = Target is immune!
        attachee.float_mesfile_line("mes\\spell.mes", spellMesId)
        args.condition_remove()
    return 0

def durationTickdown(attachee, args, evt_obj):
    duration = args.get_arg(0)
    duration -= evt_obj.data1
    args.set_arg(0, duration)
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def sickenedPenalty(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    conditionTag = "TAG_{}".format(conditionName.upper().replace(" ", "_"))
    bonus = -2 #Sickened is a -2 penalty to attack rolls, saving throws, skill checks, and ability checks.
    bonusType = bonus_type_untyped # Stacking!
    evt_obj.bonus_list.add(bonus, bonusType, "~{}~[{}]".format(conditionName, conditionTag))
    return 0

def queryIsSickened(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def getDurationLabel(duration):
    if duration == 1:
        return "1 round"
    return "{} rounds".format(duration)

def sickenedTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    duration = args.get_arg(0)
    durationLabel = getDurationLabel(duration)
    evt_obj.append("{} ({})".format(conditionName, durationLabel))
    return 0

def sickenedEffectTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    conditionKey = conditionName.upper().replace(" ", "_")
    duration = args.get_arg(0)
    durationLabel = getDurationLabel(duration)
    evt_obj.append(tpdp.hash(conditionKey), -2, " ({})".format(durationLabel))
    return 0

def removeParticles(attachee, args, evt_obj):
    particlesId = args.get_arg(1)
    if particlesId:
        game.particles_end(particlesId)
    return 0

SickenedCondition = PythonModifier("Sickened", 3, False) #duration, particlesId, empty
SickenedCondition.AddHook(ET_OnConditionAddPre, EK_NONE, checkConditionalRemoval, ())
SickenedCondition.AddHook(ET_OnConditionAdd, EK_NONE, checkImmunity, ())
SickenedCondition.AddHook(ET_OnBeginRound, EK_NONE, durationTickdown, ())
SickenedCondition.AddHook(ET_OnToHitBonus2, EK_NONE, sickenedPenalty,())
SickenedCondition.AddHook(ET_OnGetSkillLevel, EK_NONE, sickenedPenalty,())
SickenedCondition.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, sickenedPenalty,())
SickenedCondition.AddHook(ET_OnSaveThrowLevel, EK_NONE, sickenedPenalty,())
SickenedCondition.AddHook(ET_OnD20PythonQuery, "PQ_Is_Sickened", queryIsSickened, ())
SickenedCondition.AddHook(ET_OnGetTooltip, EK_NONE, sickenedTooltip, ())
SickenedCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, sickenedEffectTooltip, ())
SickenedCondition.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())

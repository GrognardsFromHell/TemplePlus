from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from spell_utils import checkCategoryType

print "Registering Shaken"

def getConditionName(args):
    return args.get_cond_name()

def getConditionTag(conditionName):
    return "TAG_{}".format(conditionName.upper().replace(" ", "_"))

def checkConditionalRemoval(attachee, args, evt_obj):
    if (evt_obj.is_modifier("sp-Remove Fear")
    or evt_obj.is_modifier("sp-Fear")
    or evt_obj.is_modifier("sp-Remove Shaken")
    or evt_obj.is_modifier("sp-Remove Panicked")):
        args.condition_remove()
    return 0

def checkImmunity(attachee, args, evt_obj):
    #check if immune to Shaken
    if checkCategoryType(attachee, mc_type_construct, mc_type_ooze, mc_type_plant, mc_type_undead, mc_type_vermin):
        spellMesId = 32000 #ID 32000 = Target is immune!
        attachee.float_mesfile_line('mes\\spell.mes', spellMesId)
        args.condition_remove()
    else:
        conditionName = getConditionName(args)
        conditionTag = getConditionTag(conditionName)
        attachee.float_text_line("{}".format(conditionName), tf_red)
        particlesId = game.particles("sp-Fear-Hit", attachee)
        args.set_arg(1, particlesId)
        game.create_history_freeform(attachee.description + " is ~{}~[{}]\n\n".format(conditionName, conditionTag))
    return 0

def durationTickdown(attachee, args, evt_obj):
    args.set_arg(0, args.get_arg(0)-evt_obj.data1) # Ticking down duration
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def shakenPenalty(attachee, args, evt_obj):
    conditionName = getConditionName(args)
    conditionTag = getConditionTag(conditionName)
    bonus = -2 #Shaken is a -2 penalty to attack rolls, saving throws and skill and ability checks.
    bonusType = 0 #ID 0 = untyped (stacking)
    evt_obj.bonus_list.add(bonus, bonusType, "~{}~[{}]".format(conditionName, conditionTag))
    return 0

def queryIsShaken(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def getDurationString(duration):
    if duration != 1:
        return "rounds"
    return "round"

def shakenTooltip(attachee, args, evt_obj):
    conditionName = getConditionName(args)
    duration = args.get_arg(0)
    durationString = getDurationString(duration)
    evt_obj.append("{} ({} {})".format(conditionName, duration, durationString))
    return 0


def shakenEffectTooltip(attachee, args, evt_obj):
    conditionName = getConditionName(args)
    conditionKey = conditionName.upper().replace(" ", "_")
    duration = args.get_arg(0)
    durationString = getDurationString(duration)
    evt_obj.append(tpdp.hash(conditionKey), -2, " ({} {})".format(duration, durationString))
    return 0

def removeParticles(attachee, args, evt_obj):
    particlesId = args.get_arg(1)
    game.particles_end(particlesId)
    return 0

shakenCondition = PythonModifier("Shaken", 3) #duration, particlesId, empty
shakenCondition.AddHook(ET_OnConditionAddPre, EK_NONE, checkConditionalRemoval, ())
shakenCondition.AddHook(ET_OnConditionAdd, EK_NONE, checkImmunity, ())
shakenCondition.AddHook(ET_OnBeginRound, EK_NONE, durationTickdown, ())
shakenCondition.AddHook(ET_OnToHitBonus2, EK_NONE, shakenPenalty,())
shakenCondition.AddHook(ET_OnGetSkillLevel, EK_NONE, shakenPenalty,())
shakenCondition.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE, shakenPenalty,())
shakenCondition.AddHook(ET_OnSaveThrowLevel, EK_NONE, shakenPenalty,())
shakenCondition.AddHook(ET_OnD20PythonQuery, "PQ_Is_Shaken", queryIsShaken, ())
shakenCondition.AddHook(ET_OnGetTooltip, EK_NONE, shakenTooltip, ())
shakenCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, shakenEffectTooltip, ())
shakenCondition.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())

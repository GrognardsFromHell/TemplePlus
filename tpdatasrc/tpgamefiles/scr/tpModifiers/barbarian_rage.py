from templeplus.pymod import PythonModifier
from toee import *
import tpdp

### This replaces Barbarian_Rage and Barbarian_Raged so PrC's and different Rage Effects
### can interact with the Barbarian Rage Class Feature

#signalId: 1 = Barbarian Rage, 2: Active Rage, 3: Greater Rage, 4: Mighty Rage

def getFeatName(signalId):
    if signalId == 1:
        combatMesId = 5045 #ID 5045: Barbarian Rage
        return game.get_mesline('mes\\combat.mes', combatMesId)
    elif signalId == 2:
        combatMesId = 5045 #ID 5045: Barbarian Rage
        mesString = game.get_mesline('mes\\combat.mes', combatMesId)
        return "End {}".format(mesString)
    elif signalId == 3:
        return "Greater Rage"
    elif signalId == 4:
        return "Mighty Rage"
    return "Error! Wrong signalId"

def getFeatTag(signalId):
    if signalId == 1 or signalId == 2:
        return "TAG_CLASS_FEATURES_BARBARIAN_RAGE"
    elif signalId == 3:
        return "TAG_CLASS_FEATURES_GREATER_RAGE"
    elif signalId == 4:
        return "TAG_CLASS_FEATURES_MIGHTY_RAGE" #help TBD!
    return "Error! Wrong signalId"

def getMaxCharges(attachee):
    classCharges = (attachee.stat_level_get(stat_level_barbarian) / 4) + 1
    chargesFromFeats = attachee.d20_query("PQ_Get_Extra_Barbarian_Rage_Charges")
    return classCharges + chargesFromFeats

def setBarbarianRageCharges(attachee, args, evt_obj):
    maxCharges = getMaxCharges(attachee)
    args.set_arg(0, maxCharges)
    return 0

#Just in case this is ever needed, I'll add the possibility now
def deductBarbarianRageCharge(attachee, args, evt_obj):
    chargesToDeduct = evt_obj.data1 or 1 #Just to be sure, if there was no data1 given in the d20_signal
    chargesLeft = args.get_arg(0)
    if chargesLeft:
        chargesLeft -= chargesToDeduct
        args.set_arg(0, chargesLeft)
    return 0

def getBarbarianRageCharges(attachee, args, evt_obj):
    chargesLeft = args.get_arg(0)
    evt_obj.return_val = chargesLeft
    return 0

def getSignalId(attachee):
    if attachee.d20_query(Q_Barbarian_Raged):
        return 2
    elif attachee.has_feat(feat_mighty_rage):
        return 4
    elif attachee.has_feat(feat_greater_rage):
        return 3
    return 1

def barbarianRageRadial(attachee, args, evt_obj):
    signalId = getSignalId(attachee)
    featName = getFeatName(signalId)
    featTag = getFeatTag(signalId)
    if signalId != 2:
        maxCharges = getMaxCharges(attachee)
        chargesLeft = args.get_arg(0)
        radialId = tpdp.RadialMenuEntryAction("{} {}/{}".format(featName, chargesLeft, maxCharges), D20A_BARBARIAN_RAGE, signalId, featTag)
        radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    else:
        radialId = tpdp.RadialMenuEntryAction(featName, D20A_BARBARIAN_RAGE, signalId, featTag)
        radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def barbarianRageCheck(attachee, args, evt_obj):
    signalId = evt_obj.d20a.data1
    if signalId != 2:
        chargesLeft = attachee.d20_query("PQ_Get_Barbarian_Rage_Charges")
        if chargesLeft < 1:
            evt_obj.return_val = AEC_OUT_OF_CHARGES
    elif signalId > 4:
        evt_obj.return_val = AEC_INVALID_ACTION
    return 0

def barbarianRagePerform(attachee, args, evt_obj):
    ### Anim_goal missing
    ### This should be moved to frame
    signalId = evt_obj.d20a.data1
    if signalId != 2:
        if signalId not in range(1,5):
            signalId = 1
        featName = getFeatName(signalId)
        featTag = getFeatTag(signalId)
        #Rage duration is 3 + Constitution Modifier, but includes the new Con Modifier from rage
        #So the Raged Condition itself will add the Con Modifier bonus to the duration to simplify things
        #Extend Rage effects will be added here
        #They get generalized in a query, so different effects can all simply interact without needing to extendExsisting
        extendDurationEffects = attachee.d20_query("PQ_Extend_Barbarian_Rage")
        duration = 3 + extendDurationEffects
        game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
        particlesId = game.particles("Barbarian Rage", attachee)
        if attachee.condition_add_with_args("{} Effect".format(featName), duration, particlesId, signalId, 0):
            # Deduct Barbarian Rage Charge
            chargesToDeduct = 1
            attachee.d20_send_signal("PS_Deduct_Barbarian_Rage_Charge", chargesToDeduct)
    else:
        #duration = 10
        #attachee.condition_add_with_args("Barbarian_Fatigued", duration, 0)
        attachee.d20_send_signal("PS_End_Barbarian_Rage")
    return 0

barbarianRageFeat = PythonModifier("Barbarian_Rage", 2) #maxCharges, empty
barbarianRageFeat.MapToFeat(feat_barbarian_rage, feat_cond_arg2 = 0)
barbarianRageFeat.AddHook(ET_OnConditionAdd, EK_NONE, setBarbarianRageCharges, ())
barbarianRageFeat.AddHook(ET_OnNewDay, EK_NEWDAY_REST, setBarbarianRageCharges, ())
barbarianRageFeat.AddHook(ET_OnD20PythonSignal, "PS_Deduct_Barbarian_Rage_Charge", deductBarbarianRageCharge, ())
barbarianRageFeat.AddHook(ET_OnD20PythonQuery, "PQ_Get_Barbarian_Rage_Charges", getBarbarianRageCharges, ())
barbarianRageFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, barbarianRageRadial, ())
barbarianRageFeat.AddHook(ET_OnD20ActionCheck, EK_D20A_BARBARIAN_RAGE, barbarianRageCheck, ())
barbarianRageFeat.AddHook(ET_OnD20ActionPerform, EK_D20A_BARBARIAN_RAGE, barbarianRagePerform, ())


### class BarbarianRagedModifier
def rageTickDown(attachee, args, evt_obj):
    duration = args.get_arg(0)
    duration -= evt_obj.data1
    args.set_arg(0, duration)
    if args.get_arg(0) < 0:
        args.condition_remove()
    return 0

def getDurationString(duration):
    if duration == 1:
        return "round"
    return "rounds"

def rageTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    duration = args.get_arg(0)
    durationString = getDurationString(duration)
    evt_obj.append("{} ({} {})".format(conditionName, duration, durationString))
    return 0

def rageEffectTooltip(attachee, args, evt_obj):
    conditionName = args.get_cond_name()
    conditionNameKey = conditionName.upper().replace(" ", "_")
    duration = args.get_arg(0)
    durationString = getDurationString(duration)
    evt_obj.append(tpdp.hash(conditionNameKey), -2, " ({} {})".format(duration, durationString))
    return 0

def removeOnUnconDead(attachee, args, evt_obj):
    if evt_obj.is_modifier("Unconscious"):
        args.condition_remove()
    elif evt_obj.is_modifier("Dead"):
        args.condition_remove()
    return 0

def updateDuration(attachee, args, evt_obj):
    #Add Constitution Modifier to duration
    duration = args.get_arg(0)
    conMod = (attachee.stat_level_get(stat_constitution) - 10) / 2
    duration += conMod
    args.set_arg(0, duration)
    return 0

def queryReturnTrue(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def removeRage(attachee, args, evt_obj):
    args.condition_remove()
    return 0

def removeRageEffects(attachee, args, evt_obj):
    particlesId = args.get_arg(1)
    game.particles_end(particlesId)
    #Query for special rage fatigue immunity like Tireless Rage
    hasRageFatigueImmunity = attachee.d20_query("PQ_Rage_Fatigue_Immunity")
    if not hasRageFatigueImmunity:
        duration = 10
        if attachee.d20_query("Fatigued"):
            attachee.d20_send_signal("Add Barbarian Fatigue", duration)
        else:
            upgradable = 1
            attachee.condition_add_with_args("FatigueExhaust", duration, duration, 0, upgradable, 0, 0)
    return 0

def barbRageAbilityBonus(attachee, args, evt_obj):
    signalId = args.get_arg(2)
    featName = getFeatName(signalId)
    featTag = getFeatTag(signalId)
    bonusValue = args.get_param(0)
    bonusType = 0 #ID 0: untyped (stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName,featTag))
    return 0

def barbRageWillSave(attachee, args, evt_obj):
    signalId = args.get_arg(2)
    featName = getFeatName(signalId)
    featTag = getFeatTag(signalId)
    bonusValue = args.get_param(0) / 2 #Bonus to will save his half the bonus to both Ability Scores
    bonusType = 0 #ID 0: untyped (stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName,featTag))
    return 0

def barbRageAcDebuff(attachee, args, evt_obj):
    signalId = args.get_arg(2)
    featName = getFeatName(signalId)
    featTag = getFeatTag(signalId)
    bonusValue = -2 #Debuff to AC is the same for all three Barbarian Rage Features
    bonusType = 0 #ID 0: untyped (stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName,featTag))
    return 0


### The BarbarianRagedModifier class contains effects common to all rage effects
### Basically forbid casting; handle duration, tooltip and particles
### ToDO: Forbid Singing!
class BarbarianRagedModifier(PythonModifier):
    #BarbarianRagedModifier have 4 args:
    #0: duration, 1: particlesId, 2: signalId, 3: empty
    def __init__(self, name):
        PythonModifier.__init__(self, name, 4, True)
        self.AddHook(ET_OnBeginRound, EK_NONE, rageTickDown, ())
        self.AddHook(ET_OnGetTooltip, EK_NONE, rageTooltip, ())
        self.AddHook(ET_OnGetEffectTooltip, EK_NONE, rageEffectTooltip, ())
        self.AddHook(ET_OnConditionAddPre, EK_NONE, removeOnUnconDead, ())
        self.AddHook(ET_OnConditionAdd, EK_NONE, updateDuration, ())
        self.AddHook(ET_OnD20Query, EK_Q_CannotCast, queryReturnTrue, ())
        self.AddHook(ET_OnConditionRemove2, EK_NONE, removeRageEffects, ())
        self.AddHook(ET_OnD20Signal, EK_S_Killed, removeRage, ())
        #HP_Changed Signal adds Fatigue if uncon/dead, but unsure if really needed
        #as this can be done by OnConditionAddPre as well????

    #Barbarian Rage Specific Effects
    #Do not add for different rage effects!
    def barbarianRageEffects(self, abilityBonus):
        self.AddHook(ET_OnD20PythonSignal, "PS_End_Barbarian_Rage", removeRage, ())
        self.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, barbRageAbilityBonus, (abilityBonus,))
        self.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CONSTITUTION, barbRageAbilityBonus, (abilityBonus,))
        self.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, barbRageWillSave, (abilityBonus,))
        self.AddHook(ET_OnGetAC, EK_NONE, barbRageAcDebuff, ())
        self.AddHook(ET_OnD20Query, EK_Q_Barbarian_Raged, queryReturnTrue, ())


### Barbarian Rage Conditions
barbRage = BarbarianRagedModifier("Barbarian Rage Effect")
barbRage.barbarianRageEffects(4) #4: abilityBonus

greaterRage = BarbarianRagedModifier("Greater Rage Effect")
greaterRage.barbarianRageEffects(6) #6: abilityBonus

mightyRage = BarbarianRagedModifier("Mighty Rage Effect")
mightyRage.barbarianRageEffects(8) #8: abilityBonus

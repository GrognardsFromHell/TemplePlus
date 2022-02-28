from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier, getSpellHelpTag, skillCheck, querySpellCondition

print "Registering sp-Evard's Black Tentacles"

### Special Grapple Check ###

def getSpecialSizeBonus(obj):
    #From the SRD:
    #The special size modifier for a grapple check is as follows:
    #Colossal +16, Gargantuan +12, Huge +8, Large +4, Medium +0, Small -4, Tiny -8, Diminutive -12, Fine -16.
    #Use this number in place of the normal size modifier you use when making an attack roll.
    size = obj.get_size
    return (size - 5) * 4

def getBonusList(obj):
    #Grapple Check is Base attack bonus + Strength modifier + special size modifier
    bonusList = tpdp.BonusList()
    baseAttackBonus = obj.get_base_attack_bonus()
    strengthBonus = (obj.stat_level_get(stat_strength) - 10) / 2
    sizeBonus = getSpecialSizeBonus(obj)
    bonusList.add(baseAttackBonus, bonus_type_untyped, 137) #Class
    bonusList.add(strengthBonus, bonus_type_stat_strength, 103) #Strength Bonus
    bonusList.add(sizeBonus, bonus_type_size, 115) #Size Modifier
    #Add special bonus modifiers
    if obj.has_feat(feat_improved_grapple):
        bonusListTarget.add(4, bonus_type_untyped, 232) #Improved Grapple Feat Bonus; there is no help for improved grapple
    #If Grapple gets added, I will add the Spell Compendium spell Fearsome Grapple as well.
    #fearsomeGrappleBonus = obj.d20_query("PQ_Fearsome_Grapple_Bonus")
    #if fearsomeGrappleBonus:
    #    bonusListTarget.add(fearsomeGrappleBonus, bonus_type_untyped, "help_string")
    return bonusList

def getTentacleBonusList(casterLevel):
    bonusList = tpdp.BonusList()
    bonusList.add(4, bonus_type_stat_strength, 103) #ID 103 = Strength Bonus
    bonusList.add(4, bonus_type_size, 115) #ID 115 = Size Modifier
    bonusList.add(casterLevel, bonus_type_untyped, "Tentacle Attack Bonus")
    return bonusList

def grappleRoll(tentacle, target, casterLevel):
    #Create bonus lists for both grapplers
    performerBonusList = getTentacleBonusList(casterLevel)
    targetBonusList = getBonusList(target)
    #Perform Grapple Check
    grappleDice = dice_new("1d20")
    performerGrappleRoll = grappleDice.roll()
    performerGrappleResult = performerGrappleRoll + performerBonusList.get_total()
    targetGrappleRoll = grappleDice.roll()
    targetGrappleResult = targetGrappleRoll + targetBonusList.get_total()
    #Set result
    if performerGrappleResult > targetGrappleResult:
        grappleSuccess = True
    elif performerGrappleResult == targetGrappleResult:
        if performerBonusList.get_total() > targetBonusList.get_total():
            grappleSuccess = True
        else:
            grappleSuccess = False
    else:
        grappleSuccess = False
    #Create History Window Entry
    combatMesTitleId = 5500 #NEW ID in combat_ext.mes
    combatMesResultId = 102 if grappleSuccess else 103 #102 = Success, 103 = Failure
    checkFlag = 1 #required
    grappleHistoryId = tpdp.create_history_type6_opposed_check(tentacle, target, performerGrappleRoll, targetGrappleRoll, performerBonusList, targetBonusList, combatMesTitleId, combatMesResultId, checkFlag)
    game.create_history_from_id(grappleHistoryId)
    return grappleSuccess

def breakFreeRoll(performer, tentacle, casterLevel):
    #Create bonus lists for both grapplers
    performerBonusList = getBonusList(performer)
    targetBonusList = getTentacleBonusList(casterLevel)
    #Perform Grapple Check
    grappleDice = dice_new("1d20")
    performerGrappleRoll = grappleDice.roll()
    performerGrappleResult = performerGrappleRoll + performerBonusList.get_total()
    targetGrappleRoll = grappleDice.roll()
    targetGrappleResult = targetGrappleRoll + targetBonusList.get_total()
    #Set result
    if performerGrappleResult > targetGrappleResult:
        grappleSuccess = True
    elif performerGrappleResult == targetGrappleResult:
        if performerBonusList.get_total() > targetBonusList.get_total():
            grappleSuccess = True
        else:
            grappleSuccess = False
    else:
        grappleSuccess = False
    #Create History Window Entry
    combatMesTitleId = 5061 #ID 5061 = Break Free
    combatMesResultId = 102 if grappleSuccess else 103 #102 = Success, 103 = Failure
    checkFlag = 1 #required
    grappleHistoryId = tpdp.create_history_type6_opposed_check(performer, tentacle, performerGrappleRoll, targetGrappleRoll, performerBonusList, targetBonusList, combatMesTitleId, combatMesResultId, checkFlag)
    game.create_history_from_id(grappleHistoryId)
    return grappleSuccess


### Start AoE Event ###

def tentacleGrappleAttempt(attachee, target, spellId):
    isMelee = 1
    touchAttack = attachee.perform_touch_attack(target, isMelee)
    if touchAttack & D20CAF_HIT:
        spellPacket = tpdp.SpellPacket(spellId)
        casterLevel = spellPacket.caster_level
        if grappleRoll(attachee, target, casterLevel):
            newSpellId = tpactions.get_new_spell_id()
            newSpellPacket = tpdp.SpellPacket(newSpellId)
            newSpellPacket.spell_enum = spell_black_tentacle_grapple
            newSpellPacket.caster = attachee
            newSpellPacket.caster_level = spellPacket.caster_level
            newSpellPacket.add_target(target, 0)
            tpactions.register_spell_cast(newSpellPacket, newSpellId)
            tpactions.trigger_spell_effect(newSpellId)
            target.float_mesfile_line("mes\\spell.mes", 21002, tf_red) #ID 21000 = Grappled!
        else:
            target.float_mesfile_line("mes\\spell.mes", 21001, tf_red) #ID 21001 = Grapple failed!
    else:
        attachee.float_mesfile_line("mes\\combat.mes", 29) #ID 29 = Miss!
    return 0

def sizeCheck(target):
    #Grapple Checks automatically fail if grappleTarget is two or more size categories larger
    #Tentacle counts as a Large Creature
    targetSize = target.get_size
    return True if targetSize < 8 else False

def grappleTriggerOnEnter(attachee, args, evt_obj):
    target = evt_obj.target
    if target.d20_query(Q_Critter_Has_Freedom_of_Movement):
        return 0
    elif sizeCheck(target):
        spellId = args.get_arg(0)
        tentacleGrappleAttempt(attachee, target, spellId)
    return 0

def grappleTriggerOnBeginRound(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    targetCount = spellPacket.target_count
    idx = 0
    for idx in range(0, targetCount):
        target = spellPacket.get_target(idx)
        if target.d20_query(Q_Critter_Has_Freedom_of_Movement):
            continue
        elif target.d20_query("PQ_Affected_By_Tentacles"):
            continue
        elif sizeCheck(target):
            tentacleGrappleAttempt(attachee, target, spellId)
        idx += 1
    return 0

def tentacleToHit(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    casterLevel = spellPacket.caster_level
    evt_obj.bonus_list.reset()
    evt_obj.bonus_list = getTentacleBonusList(casterLevel)
    return 0

blackTentaclesSpell = AoeSpellHandleModifier("sp-Evard's Black Tentacles") #spellId, duration, spellDc, eventId, empty
blackTentaclesSpell.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, grappleTriggerOnEnter, ())
blackTentaclesSpell.AddHook(ET_OnBeginRound, EK_NONE, grappleTriggerOnBeginRound, ())
blackTentaclesSpell.AddHook(ET_OnToHitBonus2, EK_NONE, tentacleToHit, ())
blackTentaclesSpell.AddSpellDismiss()

### Start Cloud Effect ###

def halfMovementSpeed(attachee, args, evt_obj):
    if not attachee.d20_query(Q_Critter_Has_Freedom_of_Movement):
        bonusValue = (attachee.stat_level_get(stat_movement_speed) / 2) * -1
        bonusType = bonus_type_untyped
        spellId = args.get_arg(0)
        spellHelpTag = getSpellHelpTag(spellId)
        evt_obj.bonus_list.add(bonusValue, bonusType , spellHelpTag)
    return 0

def checkFreedomOfMovement(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Freedom of Movement"):
        args.remove_spell()
        args.remove_spell_mod()
    return 0

blackTentaclesEffect = AoeSpellEffectModifier("Evard's Black Tentacles") #spellId, duration, spellDc, eventId, empty
blackTentaclesEffect.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, halfMovementSpeed, ())
blackTentaclesEffect.AddHook(ET_OnConditionAddPre, EK_NONE, checkFreedomOfMovement, ())

### Start Grapple Effect ###

def radialBreakFree(attachee, args, evt_obj):
    if attachee.d20_query(Q_Is_BreakFree_Possible):
        spellId = args.get_arg(0)
        radialName = game.get_mesline("mes\\combat.mes", 5061) # ID 5061 = Break Free
        radialHelpTag = "TAG_RADIAL_MENU_BREAK_FREE"
        radialId = tpdp.RadialMenuEntryAction(radialName, D20A_BREAK_FREE, spellId, radialHelpTag)
        radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Movement)
    return 0

def beginRoundActions(attachee, args, evt_obj):
    attachee.float_mesfile_line("mes\\spell.mes", 21002, tf_red) #ID 21002 = Grappled!
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    tentacle = spellPacket.caster
    if grappleRoll(tentacle, attachee, spellId):
        damageDice = dice_new("1d6")
        damageDice.bonus = 4
        attachee.damage(OBJ_HANDLE_NULL, D20DT_BLUDGEONING, damageDice, D20DAP_NORMAL, D20A_GRAPPLE)
    return 0

def limitMovementSpeed(attachee, args, evt_obj):
    evt_obj.bonus_list.set_overall_cap(1, 0, 0, 88)
    evt_obj.bonus_list.set_overall_cap(2, 0, 0, 88)
    return 0

def spellFailureCheck(attachee, args, evt_obj):
    spellPacket = evt_obj.get_spell_packet()
    requiredComponents = spellPacket.get_spell_component_flags()
    spellEnum = spellPacket.spell_enum
    spellEntry = tpdp.SpellEntry(spellEnum)
    spellCastingTime = spellEntry.casting_time
    if requiredComponents & SCF_SOMATIC:
        evt_obj.return_val = 100
    elif spellCastingTime == 1: #spellCastingTime of 1 indicates a full round action. (0 is a standard action, 4 is a swift action)
        evt_obj.return_val = 100
    else:
        spellLevel = spellPacket.spell_known_slot_level
        skillCheckDc = 20 + spellLevel
        if not skillCheck(attachee, skill_concentration, skillCheckDc):
            evt_obj.return_val = 100
    if evt_obj.return_val == 100:
        attachee.float_text_line("Spell Failure", tf_red)
        game.particles('Fizzle', attachee)
    return 0

def onSignalBreakFree(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    tentacle = spellPacket.caster
    if breakFreeRoll(attachee, tentacle, spellPacket.caster_level):
        attachee.float_mesfile_line("mes\\spell.mes", 21003) #ID 21003 = Escaped!
        #attachee.condition_add_with_args("Tentacle Immune", 0, 0)
        args.remove_spell_mod()
        args.remove_spell()
    else:
        attachee.float_mesfile_line("mes\\combat.mes", 103, tf_red) #ID 103 = Failure
    return 0

def denyDexterityBonus(attachee, args, evt_obj):
    capValue = 0
    bonusType = bonus_type_stat_dexterity
    mesLineId = 232 #Grapple
    evt_obj.bonus_list.add_cap(bonusType, capValue, mesLineId)
    return 0

def applyToHitPenalty(attachee, args, evt_obj):
    bonusValue = -4
    bonusType = bonus_type_untyped
    mesLineId = 232 #Grapple
    evt_obj.bonus_list.add(bonusValue, bonusType, mesLineId)
    return 0

def answerQueryTrue(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def answerQueryFalse(attachee, args, evt_obj):
    evt_obj.return_val = 0
    return 0

def endGrapple(attachee, args, evt_obj):
    args.remove_spell()
    args.remove_spell_mod()
    return 0
def tentacleTooltip(attachee, args, evt_obj):
    evt_obj.append("Tentacle Grapple")
    return 0

def tentacleEffectTooltip(attachee, args, evt_obj):
    tooltipTag = "TENTACLE_GRAPPLE"
    tooltipKey = tpdp.hash(tooltipTag)
    evt_obj.append(tooltipKey, -2, "")
    return 0

blackTentaclesGrapple = PythonModifier("sp-Tentacle Grapple", 3) #spellId, empty, empty
blackTentaclesGrapple.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialBreakFree, ())
blackTentaclesGrapple.AddHook(ET_OnD20Query, EK_Q_Is_BreakFree_Possible, answerQueryTrue, ())
blackTentaclesGrapple.AddHook(ET_OnBeginRound, EK_NONE, beginRoundActions, ())
blackTentaclesGrapple.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, limitMovementSpeed, ())
#blackTentaclesGrapple.AddHook(ET_OnGetMoveSpeed, EK_NONE, limitMovementSpeed, ())
blackTentaclesGrapple.AddHook(ET_OnGetAC, EK_NONE, denyDexterityBonus, ())
blackTentaclesGrapple.AddHook(ET_OnToHitBonus2, EK_NONE, applyToHitPenalty, ())
blackTentaclesGrapple.AddHook(ET_OnD20Query, EK_Q_SneakAttack, answerQueryTrue, ())
blackTentaclesGrapple.AddHook(ET_OnD20Query, EK_Q_AOOPossible, answerQueryFalse, ())
blackTentaclesGrapple.AddHook(ET_OnD20Query, EK_Q_SpellInterrupted, spellFailureCheck,())
blackTentaclesGrapple.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Grappling, answerQueryTrue, ())
blackTentaclesGrapple.AddHook(ET_OnD20PythonQuery, "PQ_Affected_By_Tentacles", answerQueryTrue, ())
blackTentaclesGrapple.AddHook(ET_OnD20Signal, EK_S_BreakFree, onSignalBreakFree, ())
blackTentaclesGrapple.AddHook(ET_OnD20Signal, EK_S_Spell_Grapple_Removed, endGrapple, ())
blackTentaclesGrapple.AddHook(ET_OnD20Signal, EK_S_Spell_End, endGrapple, ())
blackTentaclesGrapple.AddHook(ET_OnD20Signal, EK_S_Combat_End, endGrapple, ())
blackTentaclesGrapple.AddHook(ET_OnD20Signal, EK_S_Killed, endGrapple, ())
blackTentaclesGrapple.AddHook(ET_OnGetTooltip, EK_NONE, tentacleTooltip, ())
blackTentaclesGrapple.AddHook(ET_OnGetEffectTooltip, EK_NONE, tentacleEffectTooltip, ())

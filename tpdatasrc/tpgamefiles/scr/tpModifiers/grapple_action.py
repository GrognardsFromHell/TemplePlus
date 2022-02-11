from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
from spell_utils import skillCheck

########## Python Action ID"s ##########
grappleEnum = 1710 #D20A_GRAPPLE seems to crash the game, in addition there is no EK_D20A_GRAPPLE
########################################

###### Grapple Check ######

def getSpecialSizeBonus(size):
    #From the SRD:
    #The special size modifier for a grapple check is as follows:
    #Colossal +16, Gargantuan +12, Huge +8, Large +4, Medium +0, Small -4, Tiny -8, Diminutive -12, Fine -16.
    #Use this number in place of the normal size modifier you use when making an attack roll.
    return (size - 5) * 4

def getBonusList(obj, size):
    #Grapple Check is Base attack bonus + Strength modifier + special size modifier
    bonusList = tpdp.BonusList()
    baseAttackBonus = obj.get_base_attack_bonus()
    strengthBonus = (obj.stat_level_get(stat_strength) - 10) / 2
    sizeBonus = getSpecialSizeBonus(size)
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

def grappleRoll(performer, target, combatMesTitleId):
    #Grapple automatically fails, if target is 2+ size categories larger
    performerSize = performer.get_size
    targetSize = target.get_size
    if performerSize + 2 <= targetSize:
        performer.float_text_line("Target to big", tf_red)
        game.create_history_freeform("{} is to big to ~grapple~[TAG_GRAPPLE]\n\n".format(target.description))
        return False
    #Create bonus lists for both grapplers
    performerBonusList = getBonusList(performer, performerSize)
    targetBonusList = getBonusList(target, targetSize)
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
    combatMesResultId = 102 if grappleSuccess else 103 #102 = Success, 103 = Failure
    checkFlag = 1 #required
    grappleHistoryId = tpdp.create_history_type6_opposed_check(performer, target, performerGrappleRoll, targetGrappleRoll, performerBonusList, targetBonusList, combatMesTitleId, combatMesResultId, checkFlag)
    game.create_history_from_id(grappleHistoryId)
    return grappleSuccess


###### Grapple Radial Action ######

def radialGrapple(attachee, args, evt_obj):
    if not attachee.d20_query(Q_Critter_Is_Grappling):
        radialName = "Grapple" #Needs to be set to appropriate mes file entry (Grapple)
        radialHelpTag = "TAG_GRAPPLE" #This help file entry is not up to date, if these changes get implemented :)
        radialData1 = 0
        #radialId = tpdp.RadialMenuEntryAction(radialName, D20A_GRAPPLE, radialData1, radialHelpTag) #this crashes the game
        radialId = tpdp.RadialMenuEntryPythonAction(radialName, D20A_PYTHON_ACTION, grappleEnum, radialData1, radialHelpTag)
        radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Offense)
    return 0

def checkGrapple(attachee, args, evt_obj):
    target = evt_obj.d20a.target
    #obj.can_melee has a bug, it returns 0 if the character could melee the target but is whild shaped
    #https://github.com/GrognardsFromHell/TemplePlus/blob/3db338ef2b290427936529989fbf540c06dcb34c/TemplePlus/combat.cpp#L411
    if not attachee.can_melee(target):
        evt_obj.return_val = AEC_TARGET_TOO_FAR
    return 0

def performGrapple(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    target = evt_obj.d20a.target
    isMelee = 1
    touchAttack = attachee.perform_touch_attack(target, isMelee)

    #anim missing

    if touchAttack & D20CAF_HIT:
        combatMesTitleId = 5500 #New ID! = Grapple Check
        if grappleRoll(attachee, target, combatMesTitleId):
            currentSequence = tpactions.get_cur_seq()
            spellPacket = currentSequence.spell_packet
            newSpellId = tpactions.get_new_spell_id()
            spellPacket.spell_enum = spell_grapple
            spellPacket.caster = attachee
            spellPacket.add_target(target, 0)
            tpactions.register_spell_cast(spellPacket, newSpellId)
            evt_obj.d20a.spell_id = newSpellId
            currentSequence.spell_action.spell_id = newSpellId
            tpactions.trigger_spell_effect(newSpellId)
            attachee.float_mesfile_line("mes\\spell.mes", 21000, tf_red) #ID 21000 = Grapple successful!
        else:
            attachee.float_mesfile_line("mes\\spell.mes", 21001, tf_red) #ID 21001 = Grapple failed!
    else:
        attachee.float_mesfile_line("mes\\combat.mes", 29) #ID 29 = Miss!

grappleAction = PythonModifier("Grapple Action", 2) #empty, empty
grappleAction.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialGrapple, ())
grappleAction.AddHook(ET_OnD20PythonActionCheck, grappleEnum, checkGrapple, ())
grappleAction.AddHook(ET_OnD20PythonActionPerform, grappleEnum, performGrapple, ())

###### sp-Grappled Condition ######

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
    target = spellPacket.caster if attachee != spellPacket.caster else spellPacket.get_target(0)
    combatMesTitleId = 5061 #ID 5061 = Break Free
    if grappleRoll(attachee, target, combatMesTitleId):
        attachee.float_mesfile_line("mes\\spell.mes", 21003) #ID 21003 = Escaped!
        target.d20_send_signal(S_Spell_Grapple_Removed)
        target.float_mesfile_line("mes\\spell.mes", 21004) #ID 21004 = Grapple Ended!
        args.remove_spell()
        args.remove_spell_mod()
    else:
        attachee.float_mesfile_line("mes\\combat.mes", 103, tf_red) #ID 103 = Failure
    return 0

def verifyGrappleParticipant(obj, spellId):
    spellPacket = tpdp.SpellPacket(spellId)
    grappleParticipants = [spellPacket.caster, spellPacket.get_target(0)]
    return True if obj in grappleParticipants else False

def sneakAttackPossible(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = args.get_arg(0)
    attacker = currentSequence.performer
    if not verifyGrappleParticipant(attacker, spellId):
        evt_obj.return_val = 1
    return 0

def denyDexterityBonus(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    attacker = evt_obj.attack_packet.attacker
    if not verifyGrappleParticipant(attacker, spellId):
        capValue = 0
        bonusType = bonus_type_stat_dexterity
        mesLineId = 232 #Grapple
        evt_obj.bonus_list.add_cap(bonusType, capValue, mesLineId)
    return 0

def applyToHitPenalty(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    target = evt_obj.attack_packet.target
    if not verifyGrappleParticipant(target, spellId):
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
    args.condition_remove() #If I do not add this, condition does not get removed from caster
    return 0

def grappledTooltip(attachee, args, evt_obj):
    evt_obj.append("Grappled")
    return 0

def grappledEffectTooltip(attachee, args, evt_obj):
    tooltipTag = "GRAPPLED_CONDITION"
    tooltipKey = tpdp.hash(tooltipTag)
    evt_obj.append(tooltipKey, -2, "")
    return 0

grappledCondition = PythonModifier("sp-Grappled", 3, False) #spellId, duration, empty
grappledCondition.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialBreakFree, ())
grappledCondition.AddHook(ET_OnD20Query, EK_Q_Is_BreakFree_Possible, answerQueryTrue, ())
grappledCondition.AddHook(ET_OnBeginRound, EK_NONE, beginRoundActions, ())
grappledCondition.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, limitMovementSpeed, ())
#grappledCondition.AddHook(ET_OnGetMoveSpeed, EK_NONE, limitMovementSpeed, ())
grappledCondition.AddHook(ET_OnGetAC, EK_NONE, denyDexterityBonus, ())
grappledCondition.AddHook(ET_OnToHitBonus2, EK_NONE, applyToHitPenalty, ())
grappledCondition.AddHook(ET_OnD20Query, EK_Q_SneakAttack, sneakAttackPossible, ())
grappledCondition.AddHook(ET_OnD20Query, EK_Q_AOOPossible, answerQueryFalse, ())
grappledCondition.AddHook(ET_OnD20Query, EK_Q_SpellInterrupted, spellFailureCheck,())
grappledCondition.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Grappling, answerQueryTrue, ())
grappledCondition.AddHook(ET_OnD20Signal, EK_S_BreakFree, onSignalBreakFree, ())
grappledCondition.AddHook(ET_OnD20Signal, EK_S_Spell_Grapple_Removed, endGrapple, ())
grappledCondition.AddHook(ET_OnD20Signal, EK_S_Combat_End, endGrapple, ())
grappledCondition.AddHook(ET_OnD20Signal, EK_S_Killed, endGrapple, ())
grappledCondition.AddHook(ET_OnGetTooltip, EK_NONE, grappledTooltip, ())
grappledCondition.AddHook(ET_OnGetEffectTooltip, EK_NONE, grappledEffectTooltip, ())

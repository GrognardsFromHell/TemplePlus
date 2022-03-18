from templeplus.pymod import PythonModifier
from toee import *
import tpdp
from utilities import *
import tpactions

########## Python Action Enums ##########
activateMinorAuraEnum = 8301
activateMajorAuraEnum = 8302
dismissMinorAuraEnum = 8303
dismissMajorAuraEnum = 8304
activateDraconicAuraEnum = 8451
activateDoubleDraconicAuraEnum = 8452
dismissDraconicAuraEnum = 8453
dismissDoubleDraconicAuraEnum = 8454
#########################################

def getMinorAuraList():
    minor_auras = [
    aura_accurate_strike,
    aura_art_of_war,
    aura_Demand_fortitude,
    aura_Determined_caster,
    aura_force_of_will,
    aura_master_of_opportunity,
    aura_master_of_tactics,
    aura_motivate_charisma,
    aura_motivate_constitution,
    aura_motivate_dexterity,
    aura_motivate_intelligence,
    aura_motivate_strength,
    aura_motivate_wisdom,
    aura_over_the_top,
    aura_watchful_eye
    ]
    return minor_auras

def getMajorAuraList():
    major_auras = [
    aura_hardy_soldiers,
    aura_motivate_ardor,
    aura_motivate_attack,
    aura_motivate_care,
    aura_motivate_urgency,
    aura_resilient_troops,
    aura_steady_hand
    ]
    return major_auras

def getDraconicAuras():
    draconic_auras = [
    aura_energy_shield,
    aura_power,
    aura_presence,
    aura_resistance,
    aura_senses,
    aura_toughness,
    aura_vigor,
    aura_break_spell_resistance,
    aura_energy,
    aura_insight,
    aura_resolve,
    aura_stamina,
    aura_swiftness
    ]
    return draconic_auras

def getAuraName(auraEnum):
    return game.get_mesline("mes\\auras.mes", auraEnum)

def getAuraTag(auraEnum):
    return game.get_mesline("mes\\auras.mes", (auraEnum + 1000))

def getLearnedAuras(attachee, auraType):
    learnedAuras = []
    if auraType == aura_type_minor:
        auraList = getMinorAuraList()
        auraTypeString = "Minor Aura"
    elif auraType == aura_type_major:
        auraList = getMajorAuraList()
        auraTypeString = "Major Aura"
    elif auraType == aura_type_draconic or auraType == aura_type_double_draconic:
        auraList = getDraconicAuras()
        auraTypeString = "Draconic Aura"
    for auraEnum in auraList:
        auraName = getAuraName(auraEnum)
        if attachee.has_feat("{} {}".format(auraTypeString, auraName)):
            learnedAuras.append(auraEnum)
    return learnedAuras

def getMinorAuraBonus(auraCaster):
    #auraBonus can't be lower than 0
    charismaValue = auraCaster.stat_level_get(stat_charisma)
    auraBonus = int((charismaValue -10)/2)
    return max(auraBonus, 0)

def getMajorAuraBonus(auraCasterLevel):
    if auraCasterLevel < 7:
        return 1
    elif auraCasterLevel < 14:
        return 2
    elif auraCasterLevel < 20:
        return 3
    return 4

def getDraconicAuraBonus(auraCaster):
    sorcLevel = auraCaster.stat_level_get(stat_level_sorcerer)
    dragonShamanLevel = auraCaster.stat_level_get(stat_level_dragon_shaman)
    if sorcLevel:
        charLevel = auraCaster.stat_level_get(stat_level)
        if charLevel < 7:
            sorcBonus = 1
        elif charLevel < 14:
            sorcBonus = 2
        elif charLevel < 20:
            sorcBonus = 3
        else:
            sorcBonus = 4
    else:
        sorcBonus = 0
    if dragonShamanLevel:
        dragonShamanBonus = 1 + (dragonShamanLevel / 5)
    else:
        dragonShamanBonus = 0
    return max(sorcBonus, dragonShamanBonus)

def getAuraTypeString(auraType):
    if auraType == aura_type_minor:
        return "Marshal Minor Aura"
    elif auraType == aura_type_major:
        return "Marshal Major Aura"
    elif auraType == aura_type_draconic or auraType == aura_type_double_draconic:
        return "Draconic Aura"
    return 0

def auraAddBonusList(attachee, args, evt_obj):
    auraType = args.get_param(0)
    auraSpellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(auraSpellId)
    auraEnum = args.get_arg(1)
    auraName = getAuraName(auraEnum)
    auraTag = getAuraTag(auraEnum)
    auraTypeString = getAuraTypeString(auraType)
    if auraType == aura_type_minor:
        auraBonus = getMinorAuraBonus(auraSpellPacket.caster)
        auraBonusType = bonus_type_marshal_aura_minor #New ID (190) to handle stacking (is Circumstance Bonus)
    elif auraType == aura_type_major:
        auraBonus = getMajorAuraBonus(auraSpellPacket.caster_level)
        auraBonusType = bonus_type_marshal_aura_major #New ID (191) to handle stacking (is Circumstance Bonus)
    elif auraType == aura_type_draconic or auraType == aura_type_double_draconic:
        auraBonus = getDraconicAuraBonus(auraSpellPacket.caster)
        auraBonusType = bonus_type_draconic_aura # new ID (192) for Draconic Auras; stacking with everything but itself
    evt_obj.bonus_list.add(auraBonus, auraBonusType, "{}: ~{}~[{}]".format(auraTypeString, auraName, auraTag))
    return 0

def getTargetsInAura(auraSpellPacket):
    targetCount = auraSpellPacket.target_count
    targetList = []
    for counter in range(0, targetCount):
        target = auraSpellPacket.get_target(counter)
        targetList.append(target)
    return targetList

# Marshal Targets are not automatically affected and need to be checked
def verifyTarget(auraTarget):
    if auraTarget.stat_level_get(stat_intelligence) < 3:
        return False
    elif auraTarget.d20_query(Q_Critter_Is_Deafened):
        return False
    return True

def removeAuraTarget(attachee, spellId):
    auraSpellPacket = tpdp.SpellPacket(spellId)
    auraSpellPacket.remove_target(attachee)
    auraSpellPacket.update_registry()
    return 0

##### class AuraModifier #####
def onLeaveAura(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    auraSpellPacket = tpdp.SpellPacket(spellId)
    auraEventId = args.get_arg(2)
    if auraEventId != evt_obj.evt_id:
        return 0
    removeAuraTarget(attachee, spellId)
    args.remove_spell_mod()
    return 0

def auraEndSignal(attachee, args, evt_obj):
    signalId = evt_obj.data1
    spellId = args.get_arg(0)
    if signalId == spellId:
        removeAuraTarget(attachee, spellId)
        args.remove_spell_mod()
    return 0

def auraTooltip(attachee, args, evt_obj):
    auraEnum = args.get_arg(1)
    auraName = getAuraName(auraEnum)
    if auraEnum in getMinorAuraList():
        evt_obj.append("Minor Aura: {}".format(auraName))
    elif auraEnum in getMajorAuraList():
        evt_obj.append("Major Aura: {}".format(auraName))
    else:
        evt_obj.append("Draconic Aura: {}".format(auraName))
    return 0

def auraEffectTooltip(attachee, args, evt_obj):
    #ToDo: Customize EffectToolTip icons and text
    auraEnum = args.get_arg(1)
    auraName = getAuraName(auraEnum)
    if auraEnum in getMinorAuraList():
        evt_obj.append(tpdp.hash("MARSHAL_MINOR_AURA"), -2, " ({})".format(auraName))
    elif auraEnum in getMajorAuraList():
        evt_obj.append(tpdp.hash("MARSHAL_MAJOR_AURA"), -2, " ({})".format(auraName))
    else:
        evt_obj.append(tpdp.hash("DRACONIC_AURA"), -2, " ({})".format(auraName))
    return 0

def auraAddPreActions(attachee, args, evt_obj):
    #Intelligence drop to 3 or less would also remove the aura
    if (evt_obj.is_modifier("Unconscious")
    or evt_obj.is_modifier("Dead")
    or evt_obj.is_modifier("sp-Deafness")):
        spellId = args.get_arg(0)
        removeAuraTarget(attachee, spellId)
        args.remove_spell_mod()
    return 0

class AuraModifier(PythonModifier):
    # AuraModifier have 5 arguments:
    # 0: auraSpellId, 1: auraEnum, 2: auraEventId, 3 + 4: empty
    def __init__(self, name):
        PythonModifier.__init__(self, name, 5, False)
        self.AddHook(ET_OnObjectEvent, EK_OnLeaveAoE, onLeaveAura, ())
        self.AddHook(ET_OnD20Signal, EK_S_Spell_End, auraEndSignal, ())
        self.AddHook(ET_OnGetTooltip, EK_NONE, auraTooltip, ())
        self.AddHook(ET_OnGetEffectTooltip, EK_NONE, auraEffectTooltip, ())
        #self.AddSpellTeleportPrepareStandard()
        #self.AddSpellTeleportReconnectStandard()

    def marshalAuraAddPreActions(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, auraAddPreActions, ())

    def addSkillBonus(self, auraType, *args):
        for skill in args:
            eventKey = skill + 20
            self.AddHook(ET_OnGetSkillLevel, eventKey, auraAddBonusList, (auraType,))

##### class AuraAoeHandlingModifier #####
def onEnterAoeAura(attachee, args, evt_obj):
    auraEventId = args.get_arg(2)
    if auraEventId != evt_obj.evt_id:
        return 0
    auraTarget = evt_obj.target
    if auraTarget.is_friendly(attachee):
        auraType = args.get_arg(3)
        auraTypeString = getAuraTypeString(auraType)
        auraSpellId = args.get_arg(0)
        auraSpellPacket = tpdp.SpellPacket(auraSpellId)
        if auraType > aura_type_major or verifyTarget(auraTarget):
            if auraSpellPacket.add_target(auraTarget, 0):
                auraEnum = args.get_arg(1)
                auraName = getAuraName(auraEnum)
                auraTarget.condition_add_with_args("{} {}".format(auraTypeString, auraName), auraSpellId, auraEnum, auraEventId, 0, 0)
    return 0

def aoeHandlerSignalEnd(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    signalId = evt_obj.data1
    if spellId == signalId:
        spellPacket = tpdp.SpellPacket(spellId)
        targetList = getTargetsInAura(spellPacket)
        for target in targetList:
            target.d20_send_signal(S_Spell_End, spellId, 0)
        args.remove_spell()
    return 0

class AuraAoeHandlingModifier(PythonModifier):
    # AuraModifier have 5 arguments:
    # 0: spellId, 1: activeAura, 2: auraEventId, 3: auraType, 4: empty
    def __init__(self, name):
        PythonModifier.__init__(self, name, 5, True)
        self.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, onEnterAoeAura, ())
        self.AddHook(ET_OnD20PythonSignal, "PS_Aura_End", aoeHandlerSignalEnd, ())
        #self.AddSpellTeleportPrepareStandard()
        #self.AddSpellTeleportReconnectStandard()
        self.AddAoESpellEndStandardHook()

##### class auraRadialModifier #####
##### used by Marshal Class for both Minor and Major Aura #####
##### and by the Draconic Aura Feats #####
def getAuraSpellEnum(auraType):
    if auraType == aura_type_minor:
        return spell_marshal_minor_aura
    elif auraType == aura_type_major:
        return spell_marshal_major_aura
    elif auraType == aura_type_draconic:
        return spell_draconic_aura
    elif auraType == aura_type_double_draconic:
        return spell_double_draconic_aura
    return 0

def getPythonEnum(auraType):
    if auraType == aura_type_minor:
        return activateMinorAuraEnum
    elif auraType == aura_type_major:
        return activateMajorAuraEnum
    elif auraType == aura_type_draconic:
        return activateDraconicAuraEnum
    elif auraType == aura_type_double_draconic:
        return activateDoubleDraconicAuraEnum
    return 0

def radialAura(attachee, args, evt_obj):
    #auraType = args.get_param(0)
    auraType = args.get_arg(1)
    learnedAuras = getLearnedAuras(attachee, auraType)
    spellClass = stat_level_sorcerer if auraType == aura_type_draconic else stat_level_marshal
    if not learnedAuras:
        return 0
    #Add the top level menu
    auraTypeString = getAuraTypeString(auraType)
    pythonEnum = getPythonEnum(auraType)
    pythonDismissEnum = pythonEnum + 2
    auraSpellEnum = getAuraSpellEnum(auraType)
    radialAuraTop = tpdp.RadialMenuEntryParent(auraTypeString)
    if auraType == aura_type_draconic and attachee.stat_level_get(stat_level_dragon_shaman) < 1:
        radialAuraTopId = radialAuraTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    elif auraType == aura_type_double_draconic:
        radialAuraTopId = radialAuraTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    else:
        radialAuraTopId = radialAuraTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    #Add aura childs
    for auraEnum in learnedAuras:
        auraName = getAuraName(auraEnum)
        auraTag = getAuraTag(auraEnum)
        auraRadialId = tpdp.RadialMenuEntryPythonAction(auraName, D20A_PYTHON_ACTION, pythonEnum, auraEnum, auraTag)
        spellData = tpdp.D20SpellData(auraSpellEnum)
        casterLevel = attachee.stat_level_get(spellClass)
        spellData.set_spell_class(spellClass)
        spellData.set_spell_level(casterLevel)
        auraRadialId.set_spell_data(spellData)
        auraRadialId.add_as_child(attachee, radialAuraTopId)
    #Add Dismiss Radial
    activeAura = args.get_arg(3)
    if activeAura:
        activeAuraName = getAuraName(activeAura)
        dismissAuraId = tpdp.RadialMenuEntryPythonAction("Dismiss {}".format(activeAuraName), D20A_PYTHON_ACTION, pythonDismissEnum, activeAura, auraTag)
        if auraType == aura_type_draconic and attachee.stat_level_get(stat_level_dragon_shaman) < 1:
            dismissAuraId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
        elif auraType == aura_type_double_draconic:
            dismissAuraId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
        else:
            dismissAuraId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def activateAura(attachee, args, evt_obj):
    # check if toggle needs to deactivate an already active Aura first
    oldSpellId = args.get_arg(2)
    if oldSpellId:
        attachee.d20_send_signal("PS_Aura_End", oldSpellId)
        args.set_arg(2, 0)
        args.set_arg(3, 0)
    # activate new Aura
    auraEnum = evt_obj.d20a.data1
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    #spellPacket.caster_level += attachee.stat_level_get(stat_marshal)
    args.set_arg(2, newSpellId)
    args.set_arg(3, auraEnum)
    tpactions.register_spell_cast(spellPacket, newSpellId)
    tpactions.trigger_spell_effect(newSpellId)
    auraName = getAuraName(auraEnum)
    auraTag = getAuraTag(auraEnum)
    attachee.float_text_line("{} activated".format(auraName))
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, auraName, auraTag))
    return 0

def queryActivatedAura(attachee, args, evt_obj):
    spellId = args.get_arg(2)
    signalId = evt_obj.data1
    if spellId == signalId:
        evt_obj.return_val = args.get_arg(3)
    return 0

def dismissAura(attachee, args, evt_obj):
    if args.get_arg(3):
        attachee.d20_send_signal("PS_Aura_End", args.get_arg(2), 0)
        #attachee.d20_send_signal(S_Spell_End, args.get_arg(2))
        args.set_arg(2, 0)
        args.set_arg(3, 0)
    return 0

def checkDeactivateMarshal(attachee, args, evt_obj):
    #Deactivate Aura if specific conditions are added
    #panicked & fascinated missing
    #Basically all conditions that prohibit communication will end the auras
    if (evt_obj.is_modifier("Unconscious")
    or evt_obj.is_modifier("Dead")
    or evt_obj.is_modifier("sp-Silence Hit")
    or evt_obj.is_modifier("Held")
    or evt_obj.is_modifier("sp-Daze")
    or evt_obj.is_modifier("Nauseated")
    or evt_obj.is_modifier("Paralyzed")
    or evt_obj.is_modifier("Stunned")):
        if args.get_arg(3):
            attachee.d20_send_signal(S_Spell_End, args.get_arg(2))
            args.set_arg(2, 0)
            args.set_arg(3, 0)
    return 0

def checkDeactivateDraconic(attachee, args, evt_obj):
    #Deactivate Aura if attachee is getting Unconscious
    if (evt_obj.is_modifier("Unconscious")
    or evt_obj.is_modifier("Dead")):
        if args.get_arg(3):
            attachee.d20_send_signal(S_Spell_End, args.get_arg(2))
            args.set_arg(2, 0)
            args.set_arg(3, 0)
    return 0

class AuraRadialModifier(PythonModifier):
    # AuraRadialModifier have 5 arguments:
    # 0: featEnum, 1: auraType, 2: spellId, 3: activeAuraEnum 4: empty
    # use MapToFeat("[Name]", feat_cond_arg2 = aura_type_minor/major/draconic)
    # to set arg2 to the aura the modifier should handle
    def __init__(self, name):
        PythonModifier.__init__(self, name, 5, True)
        self.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialAura, ())
        self.AddHook(ET_OnD20PythonQuery, "PQ_Activated_Aura", queryActivatedAura, ())
        self.AddHook(ET_OnNewDay, EK_NEWDAY_REST, dismissAura, ())
        self.AddHook(ET_OnD20Signal, EK_S_Teleport_Prepare, dismissAura, ())

        # Add aura specific fuction to the Modifier
    def marshalMinorAuraActions(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, checkDeactivateMarshal, ())
        self.AddHook(ET_OnD20PythonActionPerform, activateMinorAuraEnum, activateAura, ())
        self.AddHook(ET_OnD20PythonActionPerform, dismissMinorAuraEnum, dismissAura, ())

    def marshalMajorAuraActions(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, checkDeactivateMarshal, ())
        self.AddHook(ET_OnD20PythonActionPerform, activateMajorAuraEnum, activateAura, ())
        self.AddHook(ET_OnD20PythonActionPerform, dismissMajorAuraEnum, dismissAura, ())

    def draconicAuraActions(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, checkDeactivateDraconic, ())
        self.AddHook(ET_OnD20PythonActionPerform, activateDraconicAuraEnum, activateAura, ())
        self.AddHook(ET_OnD20PythonActionPerform, dismissDraconicAuraEnum, dismissAura, ())

    def doubleDraconicAuraActions(self):
        self.AddHook(ET_OnConditionAddPre, EK_NONE, checkDeactivateDraconic, ())
        self.AddHook(ET_OnD20PythonActionPerform, activateDoubleDraconicAuraEnum, activateAura, ())
        self.AddHook(ET_OnD20PythonActionPerform, dismissDoubleDraconicAuraEnum, dismissAura, ())

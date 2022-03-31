from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import tpactions
from spell_utils import getSpellClassCode, queryActiveSpell, spellKilled, replaceCondition, spellTime

###################################################

def GetConditionName():
    return "Warlock"
    
print "Registering " + GetConditionName()

classEnum = stat_level_warlock
classSpecModule = __import__('class033_warlock')

###################################################

########## Python Action ID's ##########
resetEldritchBlastEnum = 3301
detectMagicEnum = 3302
fiendishResilienceEnum = 3303
########################################

#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
    classLvl = attachee.stat_level_get(classEnum)
    babvalue = game.get_bab_for_class(classEnum, classLvl)
    evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
    return 0

def OnGetSaveThrowFort(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Fortitude)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowReflex(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Reflex)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowWill(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Will)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0


classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())

def getSpellName(spellEnum):
    return game.get_spell_mesline(spellEnum)

#### Warlock Eldritch Essence Stance Class and Handling ####

def queryStance(attachee, args, evt_obj):
    stanceEnum = args.get_arg(0)
    evt_obj.return_val = stanceEnum
    return 0

def updateEssenceStance(attachee, args, evt_obj):
    for spellEnum in range(spell_frightful_blast, spell_penetrating_blast + 1):
        modifierName = getSpellName(spellEnum)
        if evt_obj.is_modifier(modifierName):
            args.condition_remove()
            break
    if evt_obj.is_modifier("Eldritch Essence"):
        args.condition_remove()
    return 0

def addParticles(attachee, args, evt_obj):
    spellEnum = args.get_arg(0)
    if spellEnum != spell_eldritch_blast:
        stanceName = getSpellName(spellEnum)
        particlesLabel = "sp-{}-held".format(stanceName)
        particlesId = game.particles(particlesLabel, attachee)
        args.set_arg(1, particlesId)
        attachee.float_text_line("{} activated".format(stanceName), tf_light_blue)
    else:
        attachee.float_text_line("Eldritch Essence resetted", tf_light_blue)
    return 0

def removeParticles(attachee, args, evt_obj):
    particlesId = args.get_arg(1)
    if particlesId:
        game.particles_end(particlesId)
    return 0

def addToolTip(attachee, args, evt_obj):
    spellEnum = args.get_arg(0)
    if spellEnum != spell_eldritch_blast:
        stanceName = getSpellName(spellEnum)
        evt_obj.append("{} active".format(stanceName))
    return 0

def queryDamageType(attachee, args, evt_obj):
    damageType = args.get_param(0)
    evt_obj.return_val = damageType
    return 0

def queryReturnTrue(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

class PythonModifierAddHook(tpdp.ModifierSpec):
    def AddHook(self, eventType, eventKey, callbackFcn, argsTuple):
        self.add_hook(eventType, eventKey, callbackFcn, argsTuple)

class EldritchBlastEssenceModifier(PythonModifierAddHook):
    #This class is used for all Eldritch Blast Essence Modifiers
    #It has at least two args: spellEnum, particlesId, empty
    def __init__(self, name, args = 3, preventDuplicate = False):
        super(PythonModifierAddHook, self).__init__(name, args, preventDuplicate)
        self.add_hook(ET_OnConditionAddPre, EK_NONE, updateEssenceStance, ())
        self.add_hook(ET_OnConditionAdd, EK_NONE, addParticles, ())
        self.add_hook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
        self.add_hook(ET_OnD20PythonQuery, "PQ_Eldritch_Esssence_Stance", queryStance, ())
        self.add_hook(ET_OnGetTooltip, EK_NONE, addToolTip, ())
    def ModifyDamageType(self, damageType):
        self.add_hook(ET_OnD20PythonQuery, "PQ_Eldritch_Blast_Damage_Type", queryDamageType, (damageType,))
    def AddQuerySecondaryTrue(self):
        self.add_hook(ET_OnD20PythonQuery, "PQ_Eldritch_Blast_Has_Secondary_Effect", queryReturnTrue, ())

eldritchEssenceCond = EldritchBlastEssenceModifier("Eldritch Essence") #spellEnum, empty

#### Warlock Eldritch Blast Secondary Effect Class ####

def ebSecTooltip(attachee, args, evt_obj):
    duration = args.get_arg(1)
    durationLabel = spellTime(duration)
    spellEnum = args.get_arg(2)
    secondaryEffectName = getSpellName(spellEnum)
    evt_obj.append("{} ({})".format(secondaryEffectName, durationLabel))
    return 0

def ebSecEffectTooltip(attachee, args, evt_obj):
    duration = args.get_arg(1)
    durationLabel = spellTime(duration)
    spellEnum = args.get_arg(2)
    secondaryEffectName = getSpellName(spellEnum)
    effectKey = secondaryEffectName.upper().replace(" ", "_")
    evt_obj.append(tpdp.hash(effectKey), -2, " ({})".format(durationLabel))
    return 0

class EldritchBlastSecondaryEffect(PythonModifierAddHook):
    #This class is used for all Eldritch Blast Secondary Effects
    #It has at least 4 args: spellId, duration, secondaryEffectEnum, empty
    def __init__(self, name, args = 4, preventDuplicate = False):
        super(PythonModifierAddHook, self).__init__(name, args, preventDuplicate)
        self.add_hook(ET_OnGetTooltip, EK_NONE, ebSecTooltip, ())
        self.add_hook(ET_OnGetEffectTooltip, EK_NONE, ebSecEffectTooltip, ())
        self.add_spell_dispel_check_standard()
        self.add_spell_countdown_standard()
        self.add_spell_teleport_prepare_standard()
        self.add_spell_teleport_reconnect_standard()
        self.add_hook(ET_OnD20Query, EK_Q_Critter_Has_Spell_Active, queryActiveSpell, ())
        self.add_hook(ET_OnD20Signal, EK_S_Killed, spellKilled, ())
    def AddSpellNoDuplicate(self):
        self.add_hook(ET_OnConditionAddPre, EK_NONE, replaceCondition, ())

#### Warlock Callbacks for other functions #####

def verifyEldritchBlastAction(spellEnum):
    return True if spellEnum in range(spell_eldritch_blast, spell_eldritch_glaive + 1) else False

#Used by Fey Power, SF + GSF Invocation and Invocation Radial
def isInvocation(spellEnum):
    return True if spellEnum in range(spell_eldritch_blast, 2400) else False #2400 needs to be replaced with the last Invocation enum once done

#### Invocation Radial Menu ####

def getInvocationLabel(invocationType):
    mappingDict = {
    1: "Least",
    3: "Lesser",
    5: "Greater",
    7: "Dark"
    }
    return mappingDict.get(invocationType)

def radialInvocations(attachee, args, evt_obj):
    knownSpells = attachee.spells_known
    radialSpellNodeId = []
    #Radial Tops
    #label = "Invocations"
    label = game.get_mesline("mes/stat.mes", classEnum + 1000) #classEnum + 1000 returns class abbreviation
    radialTop = tpdp.RadialMenuEntryParent(label)
    radialTopId = radialTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Spells)
    radialTopBlast = tpdp.RadialMenuEntryParent("Eldritch Blast")
    radialTopBlastId = radialTopBlast.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    #Create Spell Level Nodes
    for invocationType in range(1, 8, 2):
        label = getInvocationLabel(invocationType)
        radialSpellNode = tpdp.RadialMenuEntryParent(label)
        radialSpellNodeId.append(radialSpellNode.add_as_child(attachee, radialTopId))
    #Add Invocations Radial Childs
    for spell in knownSpells:
        spellEnum = spell.spell_enum
        spellLevel = spell.spell_level
        if not isInvocation(spellEnum):
            continue
        radialSpellId = tpdp.RadialMenuEntryAction(spell)
        if spellEnum in range(spell_eldritch_blast, spell_eldritch_glaive + 1):
            radialSpellId.add_as_child(attachee, radialTopBlastId)
        else:
            if spellLevel < 3:
                radialSpellId.add_as_child(attachee, radialSpellNodeId[0])
            elif spellLevel < 5:
                radialSpellId.add_as_child(attachee, radialSpellNodeId[1])
            elif spellLevel < 7:
                radialSpellId.add_as_child(attachee, radialSpellNodeId[2])
            else:
                radialSpellId.add_as_child(attachee, radialSpellNodeId[3])
    return 0

classSpecObj.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialInvocations, ())

#### Warlock Class Feats ####

## Eldritch Blast

def getActiveEldritchEssence(attachee):
    essenceEnum = attachee.d20_query("PQ_Eldritch_Esssence_Stance")
    return essenceEnum if essenceEnum else spell_eldritch_blast

def getEldritchBlastSpellLevel(spellClass):
    spellEnum = getActiveEldritchEssence(attachee)
    spellEntry = tpdp.SpellEntry(spellEnum)
    #blastSpellLevel = min(attachee.stat_level_get(stat_level_warlock) / 2, 9) this was erratad, the spell level of an unmodified Eldritch Blast is always 1
    return spellEntry.level_for_spell_class(spellClass)

def radialResestEldritchBlast(attachee, args, evt_obj):
    essenceEnum = getActiveEldritchEssence(attachee)
    if essenceEnum != spell_eldritch_blast:
        radialName = "Reset Eldritch Blast"
        radialHelpTag = "TAG_CLASS_FEATURES_WARLOCK_ELDRITCH_BLAST"
        radialId = tpdp.RadialMenuEntryPythonAction(radialName, D20A_PYTHON_ACTION, resetEldritchBlastEnum, 0, radialHelpTag)
        radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def performResetEldritchBlast(attachee, args, evt_obj):
    attachee.condition_add_with_args("Eldritch Essence", spell_eldritch_blast, 0)
    return 0

#Spell Level of Eldritch Blast is dynamically:
#Basic Eldritch Blast = 1, but if any shape or essence modifications are
#Applied to the Blast, the highest spell level of those will set the spell level
#And therefor apply to spell DC and spell Penetration
#This is my idea on how to apply this as there is no hook ET_OnGetSpellLevelMod
def applyEldritchBlastSpellLevel(attachee, args, evt_obj):
    spellEntry = evt_obj.spell_entry
    spellEnum = spellEntry.spell_enum
    if spellEnum in range(spell_eldritch_blast, spell_eldritch_glaive + 1):
        spellClass = getSpellClassCode(classEnum)
        shapeSpellLevel = spellEntry.level_for_spell_class(spellClass)
        activeEssenceEnum = getActiveEldritchEssence(attachee)
        spellEntry = tpdp.SpellEntry(activeEssenceEnum)
        spellLevel = spellEntry.level_for_spell_class(spellClass)
        spellLevel -= shapeSpellLevel
        if spellLevel > 0:
            evt_obj.bonus_list.add(spellLevel, bonus_type_untyped, "Warlock Eldritch Blast spell level modification")
    return 0

classSpecObj.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialResestEldritchBlast, ())
classSpecObj.AddHook(ET_OnD20PythonActionPerform, resetEldritchBlastEnum, performResetEldritchBlast, ())
classSpecObj.AddHook(ET_OnGetSpellDcMod, EK_NONE, applyEldritchBlastSpellLevel, ())
classSpecObj.AddHook(ET_OnGetSpellResistanceMod, EK_NONE, applyEldritchBlastSpellLevel, ())
 
## Detect Magic SLA ##
def radialDetectMagic(attachee, args, evt_obj):
    spellEnum = spell_detect_magic
    casterLevel = attachee.stat_level_get(classEnum)
    radialName = "Detect Magic (at Will)"
    radialHelpTag = "TAG_CLASS_FEATURES_WARLOCK_DETECT_MAGIC"

    detectMagicId = tpdp.RadialMenuEntryPythonAction(radialName, D20A_PYTHON_ACTION, detectMagicEnum, spellEnum, radialHelpTag)
    spellData = tpdp.D20SpellData(spellEnum)
    spellData.set_spell_class(classEnum)
    spellData.set_spell_level(0)
    detectMagicId.set_spell_data(spellData)
    detectMagicId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def performDetectMagic(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    tpactions.register_spell_cast(spellPacket, newSpellId)
    currentSequence.spell_packet.spell_id = newSpellId

    if attachee.anim_goal_throw_spell_w_cast_anim(): # note: the animation goal has internal calls to trigger_spell_effect and the action frame
        new_anim_id = attachee.anim_goal_get_new_id()
        evt_obj.d20a.flags |= D20CAF_NEED_ANIM_COMPLETED
        evt_obj.d20a.anim_id = new_anim_id
    return 0

warlockDetectMagic = PythonModifier("Warlock Detect Magic", 2) #featEnum, empty
warlockDetectMagic.MapToFeat("Warlock Detect Magic", feat_cond_arg2 = 0)
warlockDetectMagic.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, radialDetectMagic, ())
warlockDetectMagic.AddHook(ET_OnD20PythonActionPerform, detectMagicEnum, performDetectMagic, ())

## Damage Reduction Cold Iron ##
def addColdIronDr(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    drValue = min((classLevel + 1) / 4, 5) #bonus capped at level 19
    evt_obj.damage_packet.add_physical_damage_res(drValue, D20DAP_COLD, 126) #ID 126 in damage.mes is DR; D20DAP_COLD = Cold Iron!!
    return 0

warlockDamageReduction = PythonModifier("Warlock Damage Reduction", 2) #featEnum, empty
warlockDamageReduction.MapToFeat("Warlock Damage Reduction", feat_cond_arg2 = 0)
warlockDamageReduction.AddHook(ET_OnTakingDamage2, EK_NONE, addColdIronDr, ())

## Deceive Item ##
# TBD!

## Fiendish Resilience ##
def radialFiendishResilience(attachee, args, evt_obj):
    chargesLeft = args.get_arg(1)
    radialName = "Fiendish Resilience ({}/1)".format(chargesLeft)
    radialHelpTag = "TAG_CLASS_FEATURES_WARLOCK_FIENDISH_RESILIENCE"
    fiendishResilienceId = tpdp.RadialMenuEntryPythonAction(radialName, D20A_PYTHON_ACTION, fiendishResilienceEnum, 114, radialHelpTag)
    fiendishResilienceId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def checkFiendishResilienceCharges(attachee, args, evt_obj):
    chargesLeft = args.get_arg(1)
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def getFastHealingAmount(classLevel):
    if classLevel < 13:
        return 1
    elif classLevel < 18:
        return 2
    return 5

def activateFiendishResilience(attachee, args, evt_obj):
    chargesLeft = args.get_arg(1)
    duration = 20 #2 min
    attachee.float_text_line("Fiendish Resilience activated")
    classLevel = attachee.stat_level_get(classEnum)
    healingAmount = getFastHealingAmount(classLevel)
    attachee.condition_add_with_args("Warlock Fiendish Resilience Effect", duration, healingAmount, 0)
    chargesLeft -= 1
    args.set_arg(1, chargesLeft)
    return 0

def resetFiendishResilienceCharges(attachee, args, evt_obj):
    args.set_arg(1, 1)
    return 0

warlockFiendishResilience = PythonModifier("Warlock Fiendish Resilience", 3) #featEnum, chargesLeft, empty
warlockFiendishResilience.MapToFeat("Warlock Fiendish Resilience", feat_cond_arg2 = 1)
warlockFiendishResilience.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, radialFiendishResilience, ())
warlockFiendishResilience.AddHook(ET_OnD20PythonActionCheck, fiendishResilienceEnum, checkFiendishResilienceCharges, ())
warlockFiendishResilience.AddHook(ET_OnD20PythonActionPerform, fiendishResilienceEnum, activateFiendishResilience, ())
warlockFiendishResilience.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetFiendishResilienceCharges, ())

def fiendishResilienceHealTick(attachee, args, evt_obj):
    healAmount = args.get_arg(1)
    duration = args.get_arg(0)

    ### workaround for heal ###
    #heal requires a dice
    healDice = dice_new("1d1")
    healDice.bonus = healAmount -1
    ### workaround end ###
    game.particles ("sp-Cure Minor Wounds", attachee)
    attachee.heal(attachee, healDice, D20A_HEAL, 0)
    attachee.healsubdual(attachee, healDice, D20A_HEAL, 0)

    # Ticking down duration
    duration -= evt_obj.data1
    if duration < 0:
        args.condition_remove()
    args.set_arg(0, duration)
    return 0

def fiendishResilienceOnConditionRemove(attachee, args, evt_obj):
    attachee.float_text_line("Fiendish Resilience end")
    return 0

def getDurationLabel(duration):
    if duration != 1:
        return "rounds"
    return "round"

def fiendishResilienceTooltip(attachee, args, evt_obj):
    duration = args.get_arg(0)
    durationLabel = getDurationLabel(duration)
    fastHealingAmount = args.get_arg(1)
    evt_obj.append("Fast Healing {} ({} {})".format(fastHealingAmount, duration, durationLabel))
    return 0

def fiendishResilienceEffectTooltip(attachee, args, evt_obj):
    duration = args.get_arg(0)
    durationLabel = getDurationLabel(duration)
    fastHealingAmount = args.get_arg(1)
    evt_obj.append(tpdp.hash("WARLOCK_FIENDISH_RESILIENCE"), -2, " {} ({} {})".format(fastHealingAmount, duration, durationLabel))
    return 0

warlockFiendishResilienceEffect = PythonModifier("Warlock Fiendish Resilience Effect", 3) #duration, healAmount, empty
warlockFiendishResilienceEffect.AddHook(ET_OnConditionAdd, EK_NONE, fiendishResilienceHealTick, ())
warlockFiendishResilienceEffect.AddHook(ET_OnBeginRound, EK_NONE, fiendishResilienceHealTick, ())
warlockFiendishResilienceEffect.AddHook(ET_OnConditionRemove, EK_NONE, fiendishResilienceOnConditionRemove, ())
warlockFiendishResilienceEffect.AddHook(ET_OnGetTooltip, EK_NONE, fiendishResilienceTooltip, ())
warlockFiendishResilienceEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, fiendishResilienceEffectTooltip, ())

## Energy Resistance ##
def addEnergyResistance(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    resistanceAmount = 5 if classLevel in range(10, 20) else 10
    energyType = args.get_param(0)
    evt_obj.damage_packet.add_damage_resistance(resistanceAmount, energyType, 124)
    return 0

warlockEnergyResistanceAcid = PythonModifier("Warlock Acid Resistance Feat", 2) #featEnum, empty
warlockEnergyResistanceAcid .MapToFeat("Warlock Energy Resistance - Acid", feat_cond_arg2 = 0)
warlockEnergyResistanceAcid .AddHook(ET_OnTakingDamage, EK_NONE, addEnergyResistance, (D20DT_ACID,))

warlockEnergyResistanceCold = PythonModifier("Warlock Cold Resistance Feat", 2) #featEnum, empty
warlockEnergyResistanceCold.MapToFeat("Warlock Energy Resistance - Cold", feat_cond_arg2 = 0)
warlockEnergyResistanceCold.AddHook(ET_OnTakingDamage, EK_NONE, addEnergyResistance, (D20DT_COLD,))

warlockEnergyResistanceElectricity = PythonModifier("Warlock Electricity Resistance Feat", 2) #featEnum, empty
warlockEnergyResistanceElectricity.MapToFeat("Warlock Energy Resistance - Electricity", feat_cond_arg2 = 0)
warlockEnergyResistanceElectricity.AddHook(ET_OnTakingDamage, EK_NONE, addEnergyResistance, (D20DT_ELECTRICITY,))

warlockEnergyResistanceFire = PythonModifier("Warlock Fire Resistance Feat", 2) #featEnum, empty
warlockEnergyResistanceFire.MapToFeat("Warlock Energy Resistance - Fire", feat_cond_arg2 = 0)
warlockEnergyResistanceFire.AddHook(ET_OnTakingDamage, EK_NONE, addEnergyResistance, (D20DT_FIRE,))

warlockEnergyResistanceSonic = PythonModifier("Warlock Sonic Resistance Feat", 2) #featEnum, empty
warlockEnergyResistanceSonic.MapToFeat("Warlock Energy Resistance - Sonic", feat_cond_arg2 = 0)
warlockEnergyResistanceSonic.AddHook(ET_OnTakingDamage, EK_NONE, addEnergyResistance, (D20DT_SONIC,))

## Imbue Item ##
# TBD!

#No spell failure in Light Armor for Warlock spells
def WarlockSpellFailure(attachee, args, evt_obj):
    #Only effects spells cast as a warlock
    if evt_obj.data1 != classEnum:
        return 0

    equip_slot = evt_obj.data2
    item = attachee.item_worn_at(equip_slot)

    if item == OBJ_HANDLE_NULL:
        return 0

    if equip_slot == item_wear_armor:
        armor_flags = item.obj_get_int(obj_f_armor_flags)
        if attachee.d20_query("Improved Armored Casting"):
            if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT) or (armor_flags == ARMOR_TYPE_MEDIUM):
                return 0
        else:
            if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT):
                return 0
    evt_obj.return_val += item.obj_get_int(obj_f_armor_arcane_spell_failure)
    return 0

warlockSpellFailure = PythonModifier("Warlock Spell Failure", 2) #featEnum, empty
warlockSpellFailure.MapToFeat("Warlock Spell Failure", feat_cond_arg2 = 0)
warlockSpellFailure.AddHook(ET_OnD20Query, EK_Q_Get_Arcane_Spell_Failure, WarlockSpellFailure, ())

### Spell casting
def OnGetBaseCasterLevel(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    classLvl = attachee.stat_level_get(classEnum)
    evt_obj.bonus_list.add(classLvl, 0, 137)
    return 0

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    classSpecModule.LevelupSpellsFinalize(attachee)
    return 0
    
def OnInitLevelupSpellSelection(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    classSpecModule.InitSpellSelection(attachee)
    return 0
    
def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
    if evt_obj.arg0 != classEnum:
        return 0
    if not classSpecModule.LevelupCheckSpells(attachee):
        evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
    return 1

classSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
classSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())

from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
import aura_utils
import tpactions

###################################################

def GetConditionName():
    return "Marshal"

print "Registering {}".format(GetConditionName())

classEnum = stat_level_marshal
classSpecModule = __import__('class083_marshal')

###################################################

########## Python Action ID's ##########
adrenalineBoostEnum = 8305
learnMinorAuraEnum = 8311 #workaround
learnMajorAuraEnum = 8312 #workaround
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

classSpecObj = PythonModifier(GetConditionName(), 4) #numberMinorAurasLearned, numberMajorAurasLearned, empty, empty
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())

#### Marshal Class Mechanics

### Learn Auras
##### Workaround until Class Features are implemented #####
## Can't be done via Bonus Feats, as there is no flag to mark feats as bonus feat only

# Minor Auras Learn
def newMinorAuraAvailible(classLevel, numberAurasLearned):
    dictKnownAuras = {
    1: 1,
    2: 1,
    3: 2,
    4: 2,
    5: 3,
    6: 3,
    7: 4,
    8: 4,
    9: 5,
    10: 5,
    11: 5,
    12: 6,
    13: 6,
    14: 6,
    15: 7,
    16: 7,
    17: 7,
    18: 7,
    19: 7,
    20: 8
    }
    return True if dictKnownAuras[classLevel] > numberAurasLearned else False

def radialLearnMinorAura(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    minorAuras = aura_utils.getMinorAuraList()
    learnedAuras = aura_utils.getLearnedAuras(attachee, aura_type_minor)
    numberAurasLearned = args.get_arg(0)
    if newMinorAuraAvailible(classLevel, numberAurasLearned):
        radialLearnTop = tpdp.RadialMenuEntryParent("Learn New Minor Aura")
        radialLearnTopId = radialLearnTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
        for auraEnum in minorAuras:
            if not auraEnum in learnedAuras:
                auraName = aura_utils.getAuraName(auraEnum)
                auraTag = aura_utils.getAuraTag(auraEnum)
                radialLearnAuraId = tpdp.RadialMenuEntryPythonAction(auraName, D20A_PYTHON_ACTION, learnMinorAuraEnum, auraEnum, auraTag)
                radialLearnAuraId.add_as_child(attachee, radialLearnTopId)
    return 0

def addNewMinorAura(attachee, args, evt_obj):
    auraEnum = evt_obj.d20a.data1
    auraName = aura_utils.getAuraName(auraEnum)
    attachee.feat_add("Minor Aura {}".format(auraName))
    attachee.float_text_line("{} learned".format(auraName))
    aurasLearned = args.get_arg(0)
    aurasLearned +=1
    args.set_arg(0, aurasLearned)
    return 0

classSpecObj.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, radialLearnMinorAura, ())
classSpecObj.AddHook(ET_OnD20PythonActionPerform, learnMinorAuraEnum, addNewMinorAura, ())

# Major Auras Learn
def newMajorAuraAvailible(classLevel, numberAurasLearned):
    dictKnownAuras = {
    1: 0,
    2: 1,
    3: 1,
    4: 1,
    5: 2,
    6: 2,
    7: 2,
    8: 2,
    9: 3,
    10: 3,
    11: 3,
    12: 3,
    13: 3,
    14: 4,
    15: 4,
    16: 4,
    17: 4,
    18: 4,
    19: 4,
    20: 5
    }
    return True if dictKnownAuras[classLevel] > numberAurasLearned else False

def radialLearnMajorAura(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    majorAuras = aura_utils.getMajorAuraList()
    learnedAuras = aura_utils.getLearnedAuras(attachee, aura_type_major)
    numberAurasLearned = args.get_arg(1)
    if newMajorAuraAvailible(classLevel, numberAurasLearned):
        radialLearnTop = tpdp.RadialMenuEntryParent("Learn New Major Aura")
        radialLearnTopId = radialLearnTop.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
        for auraEnum in majorAuras:
            if not auraEnum in learnedAuras:
                auraName = aura_utils.getAuraName(auraEnum)
                auraTag = aura_utils.getAuraTag(auraEnum)
                radialLearnAuraId = tpdp.RadialMenuEntryPythonAction(auraName, D20A_PYTHON_ACTION, learnMajorAuraEnum, auraEnum, auraTag)
                radialLearnAuraId.add_as_child(attachee, radialLearnTopId)
    return 0

def addNewMajorAura(attachee, args, evt_obj):
    auraEnum = evt_obj.d20a.data1
    auraName = aura_utils.getAuraName(auraEnum)
    attachee.feat_add("Major Aura {}".format(auraName))
    attachee.float_text_line("{} learned".format(auraName))
    aurasLearned = args.get_arg(1)
    aurasLearned +=1
    args.set_arg(1, aurasLearned)
    return 0

classSpecObj.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, radialLearnMajorAura, ())
classSpecObj.AddHook(ET_OnD20PythonActionPerform, learnMajorAuraEnum, addNewMajorAura, ())

##### Workaround Learn Auras end #####

## Auras
marshalMinorAura = aura_utils.AuraRadialModifier("Marshal Minor Aura Feat")
marshalMinorAura.MapToFeat("Marshal Minor Auras", feat_cond_arg2 = aura_type_minor)
marshalMinorAura.marshalMinorAuraActions()

marshalMajorAura = aura_utils.AuraRadialModifier("Marshal Major Aura Feat")
marshalMajorAura.MapToFeat("Marshal Major Auras", feat_cond_arg2 = aura_type_major)
marshalMajorAura.marshalMajorAuraActions()

## Adrenaline Boost Alternate Class Feature
def adrenalineBoostRadial(attachee, args, evt_obj):
    #Add the top level menu
    radialAdrenalineId = tpdp.RadialMenuEntryPythonAction("Adrenaline Boost ({}/{})".format(args.get_arg(1), args.get_arg(2)), D20A_PYTHON_ACTION, adrenalineBoostEnum, 0, "TAG_CLASS_FEATURES_MARSHAL_ADRENALINE_BOOST")
    spellData = tpdp.D20SpellData(spell_marshal_adrenaline_boost)
    casterLevel = attachee.stat_level_get(classEnum)
    spellData.set_spell_class(classEnum)
    spellData.set_spell_level(casterLevel)
    radialAdrenalineId.set_spell_data(spellData)
    radialAdrenalineId.add_as_child(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def resetChargesToMax(attachee, args, evt_obj):
    classLevel = attachee.stat_level_get(classEnum)
    maxUses = classLevel/4
    args.set_arg(1, maxUses)
    args.set_arg(2, maxUses)
    return 0

def checkChargesAdrenalineBoost(attachee, args, evt_obj):
    chargesLeft = args.get_arg(1)
    silencedCondRef = tpdp.get_condition_ref("sp-Silence Hit")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    elif attachee.d20_query_with_data(Q_Critter_Has_Condition, conRef, 0):
        evt_obj.return_val = AEC_INVALID_ACTION #replace with new aec_silenced
    return 0

def activateAdrenalineBoost(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    newSpellId = tpactions.get_new_spell_id()
    tpactions.register_spell_cast(spellPacket, newSpellId)
    tpactions.trigger_spell_effect(newSpellId)
    chargesLeft = args.get_arg(1)
    chargesLeft -= 1
    args.set_arg(1, chargesLeft)
    return 0

adrenalineBoost = PythonModifier("Adrenaline Boost Feat", 4) #featEnum, chargesLeft, maxUses, empty
adrenalineBoost.MapToFeat("Marshal Adrenaline Boost")
adrenalineBoost.AddHook(ET_OnBuildRadialMenuEntry , EK_NONE, adrenalineBoostRadial, ())
adrenalineBoost.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetChargesToMax, ())
adrenalineBoost.AddHook(ET_OnConditionAdd, EK_NONE, resetChargesToMax, ())
adrenalineBoost.AddHook(ET_OnD20PythonActionCheck, adrenalineBoostEnum, checkChargesAdrenalineBoost, ())
adrenalineBoost.AddHook(ET_OnD20PythonActionPerform, adrenalineBoostEnum, activateAdrenalineBoost, ())

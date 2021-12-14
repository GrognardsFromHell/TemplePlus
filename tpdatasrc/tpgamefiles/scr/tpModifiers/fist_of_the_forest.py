from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils
from barbarian_rage import BarbarianRagedModifier
import feat_utils

###################################################

def GetConditionName():
    return "Fist of the Forest"

print "Registering " + GetConditionName()

classEnum = stat_level_fist_of_the_forest
###################################################

########## Python Action ID's ##########
feralTranceEnum = 8701
biteEnum = 8702
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

#### Fist of the Forest Class Mechanics

def getClassFeatureTag(featName):
    return "TAG_CLASS_FEATURES_{}".format(featName.upper().replace(" ", "_"))

### AC Bonus
def addAcBonus(attachee, args, evt_obj):
    #Check if violated Primal Living:
    if attachee.d20_query("PQ_Has_Violated_Primal_Living"):
        evt_obj.bonus_list.add_zeroed(400) #ID 400 NEW!
    #Check if unencumbered
    #There seems to be no constants for load; 1 is light, 2 = medium, 3 = heavey, 4 = overburdened
    #Maybe add a constants block for load?
    elif attachee.stat_level_get(stat_load) > 1:
        evt_obj.bonus_list.add_zeroed(401) #ID 401 NEW!
    #check if not wearing shield
    elif attachee.item_worn_at(item_wear_shield) != OBJ_HANDLE_NULL:
        evt_obj.bonus_list.add_zeroed(402) #ID 402 NEW!
    #check if not wearing armor
    elif attachee.item_worn_at(item_wear_armor).item_d20_query(Q_Armor_Get_AC_Bonus):
        evt_obj.bonus_list.add_zeroed(403) #ID 403 NEW!
    #check if helpless
    elif attachee.d20_query(Q_Helpless):
        evt_obj.bonus_list.add_zeroed(404) #ID 404 NEW!
    else:
        featName = feat_utils.getFeatName(args)
        featTag = getClassFeatureTag(featName)
        bonusValue = feat_utils.getAbilityModifier(attachee, stat_constitution)
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName, featTag))
    return 0

fifoAcBonus = PythonModifier("Fist of the Forest AC Bonus", 2) #featEnum, empty
fifoAcBonus.MapToFeat("Fist of the Forest Ac Bonus", feat_cond_arg2 = 0)
fifoAcBonus.AddHook(ET_OnGetAC, EK_NONE, addAcBonus, ())

### Fast Movement
def addMovementSpeed(attachee, args, evt_obj):
    armorFlags = attachee.item_worn_at(item_wear_armor).obj_get_int(obj_f_armor_flags)
    #Check if violated Primal Living:
    if attachee.d20_query("PQ_Has_Violated_Primal_Living"):
        evt_obj.bonus_list.add_zeroed(400) #ID 400 NEW!
    #Check if heavy loaded
    elif attachee.stat_level_get(stat_load) > 2:
        evt_obj.bonus_list.add_zeroed(405) #ID 405 NEW!
    #check if wearing heavy armor
    elif armorFlags != ARMOR_TYPE_NONE and armorFlags > ARMOR_TYPE_MEDIUM and attachee.item_worn_at(item_wear_armor) != OBJ_HANDLE_NULL:
        evt_obj.bonus_list.add_zeroed(406) #ID 406 NEW!
    else:
        featName = feat_utils.getFeatName(args)
        featTag = getClassFeatureTag(featName)
        bonusValue = 10
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName, featTag))
    return 0

fifoFastMovement = PythonModifier("Fist of the Forest Fast Movement", 2) #featEnum, empty
fifoFastMovement.MapToFeat("Fist of the Forest Fast Movement", feat_cond_arg2 = 0)
fifoFastMovement.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, addMovementSpeed, ())

### Feral Trance
def getMaxCharges(attachee):
    if attachee.stat_level_get(classEnum) < 3:
        return 1
    return 2

def deductCharge(args):
    chargesLeft = args.get_arg(1)
    chargesLeft -= 1
    args.set_arg(1, chargesLeft)
    return 0

def resetCharges(attachee, args, evt_obj):
    maxCharges = getMaxCharges(attachee)
    args.set_arg(1, maxCharges)
    return 0

def feralTranceRadial(attachee, args, evt_obj):
    maxCharges = getMaxCharges(attachee)
    chargesLeft = args.get_arg(1)
    if attachee.d20_query("PQ_Has_Violated_Primal_Living"):
        return 0
    featName = feat_utils.getFeatName(args)
    featTag = getClassFeatureTag(featName)
    radialId = tpdp.RadialMenuEntryPythonAction("Feral Trance ({}/{})".format(chargesLeft, maxCharges), D20A_PYTHON_ACTION, feralTranceEnum, 0, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def feralTranceCheck(attachee, args, evt_obj):
    if args.get_arg(1) < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    if attachee.d20_query("PQ_Feral_Trance_Active"):
        evt_obj.return_val = AEC_ALREADY_ACTIVE
    return 0

def feralTrancePerform(attachee, args, evt_obj):
    deductCharge(args)
    #Below should be moved to Frame
    #And an anim_goal for barbarian_rage should be added
    duration = 3 #duration is 3 + constitution modifier. conMod is handled by rage condition
    particlesId = game.particles("cl-Feral Trance", attachee)
    signalId = 0 #only barbarian_rage uses signalId's
    attachee.condition_add_with_args("Feral Trance Effect", duration, particlesId, signalId, 0)
    return 0

def feralTranceFrame(attachee, args, evt_obj):
    deductCharge(args)
    particlesId = game.particles("cl-Feral Trance", attachee)
    duration = 3 #duration is 3 + constitution modifier. conMod is handled by rage condition
    attachee.condition_add_with_args("Feral Trance Effect", duration, particlesId, 0, 0)
    return 0

fifoFeralTrance = PythonModifier("Fist of the Forest Feral Trance", 3) #featEnum, chargesLeft, empty
fifoFeralTrance.MapToFeat("Fist of the Forest Feral Trance", feat_cond_arg2 = 1)
fifoFeralTrance.AddHook(ET_OnNewDay, EK_NEWDAY_REST, resetCharges, ())
fifoFeralTrance.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, feralTranceRadial, ())
fifoFeralTrance.AddHook(ET_OnD20PythonActionCheck, feralTranceEnum, feralTranceCheck, ())
fifoFeralTrance.AddHook(ET_OnD20PythonActionPerform, feralTranceEnum, feralTrancePerform, ())
#fifoFeralTrance.AddHook(ET_OnD20PythonActionFrame, feralTranceEnum, feralTranceFrame, ())

def dexBonus(attachee, args, evt_obj):
    featName = "Fist of the Forest Feral Trance"
    featTag = getClassFeatureTag(featName)
    bonusValue = 4
    bonusType = 0 # ID 0 = Untyped (stacking)
    evt_obj.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName, featTag))
    return 0

def damageBonus(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if  weaponUsed == OBJ_HANDLE_NULL:
        featName = "Fist of the Forest Feral Trance"
        featTag = getClassFeatureTag(featName)
        bonusValue = 2
        bonusType = 0 # ID 0 = Untyped (stacking)
        evt_obj.damage_packet.bonus_list.add(bonusValue, bonusType, "~{}~[{}]".format(featName, featTag))
    return 0

#full attack bonus bite attack TBD
#feralTranceEffect.AddHook(ET_OnGetBonusAttacks, EK_NONE, getBonusBiteAttack, ())
#feralTranceEffect.AddHook(ET_OnGetCritterNaturalAttacksNum, EK_NONE, testHook, ())

#Fist of the Forest Feral Trance would destroy 
#equippment in the gloves and boots slot
#I don't think this would be a good idea
#so they only get de-equipped and can't be
#re-equipped for the duration of the trance
def unequippGlovesBootsSlot(attachee, args, evt_obj):
    dropFlag = 0
    if attachee.item_worn_at(item_wear_gloves) != OBJ_HANDLE_NULL:
        attachee.item_worn_unwield(item_wear_gloves, dropFlag)
    if attachee.item_worn_at(item_wear_boots) != OBJ_HANDLE_NULL:
        attachee.item_worn_unwield(item_wear_boots, dropFlag)
    return 0

def testHook(attachee, args, evt_obj):
    print "testHook"
    return 0

feralTranceEffect = BarbarianRagedModifier("Feral Trance Effect") #duration, particlesId, signalId, empty
feralTranceEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_DEXTERITY, dexBonus, ())
feralTranceEffect.AddHook(ET_OnDealingDamage2, EK_NONE, damageBonus, ())
feralTranceEffect.AddHook(ET_OnD20PythonQuery, "PQ_Feral_Trance_Active", feat_utils.queryReturnOne, ())
feralTranceEffect.AddHook(ET_OnConditionAdd, EK_NONE, unequippGlovesBootsSlot, ())
feralTranceEffect.AddHook(ET_OnD20Signal, EK_S_Inventory_Update, unequippGlovesBootsSlot, ())
feralTranceEffect.AddHook(ET_OnGetCritterNaturalAttacksNum, EK_NONE, testHook, ())

### Primal Living
def verifyCoC(attachee, args, evt_obj):
    isOutdoor = game.is_outdoor()
    violationCount = args.get_arg(1)
    currentMonth = game.time.time_game_in_months(game.time)
    monthOfViolation = args.get_arg(2)
    if attachee.d20_query("PQ_Has_Violated_Primal_Living"):
        return 0
    elif isOutdoor and violationCount == 0:
        return 0
    elif isOutdoor and violationCount:
        if currentMonth != monthOfViolation:
            args.set_arg(1, 0)
            args.set_arg(2, 0)
        return 0
    elif not isOutdoor:
        if currentMonth != monthOfViolation:
            args.set_arg(1, 1)
            args.set_arg(2, currentMonth)
            attachee.float_text_line("Violated Primal Living!", tf_red)
        else:
            if violationCount < 3:
                violationCount += 1
                args.set_arg(1, violationCount)
                attachee.float_text_line("Violated Primal Living!", tf_red)
            else:
                args.set_arg(1, 0)
                args.set_arg(2, 0)
                attachee.float_text_line("Violated Primal Living too often!", tf_red)
                dayOfViolation = game.time.time_in_game_in_days()
                attachee.condition_add_with_args("Violated Primal Living", dayOfViolation, 0)
    return 0

def violationWarningTooltip(attachee, args, evt_obj):
    violationCount = args.get_arg(1)
    if violationCount:
        featName = feat_utils.getFeatName(args)
        featKey = featName.upper().replace(" ", "_")
        violationString = "times" if violationCount != 1 else "time"
        evt_obj.append(tpdp.hash(featKey), -2, " (violated {} {}!)".format(violationCount, violationString))
    return 0

fifoPrimalLiving = PythonModifier("Fist of the Forest Primal Living", 4) #featEnum, violationCount, monthOfViolation, empty
fifoPrimalLiving.MapToFeat("Fist of the Forest Primal Living", feat_cond_arg2 = 0)
fifoPrimalLiving.AddHook(ET_OnNewDay, EK_NEWDAY_REST, verifyCoC, ())
fifoPrimalLiving.AddHook(ET_OnGetEffectTooltip, EK_NONE, violationWarningTooltip, ())

def countdownToRedemption(attachee, args, evt_obj):
    currentDayCount = game.time.time_in_game_in_days()
    dayOfViolation = args.get_arg(0)
    if game.is_outdoor():
        if currentDayCount > dayOfViolation + 29:
            args.condition_remove()
    else:
        dayOfViolation = currentDayCount
        args.set_arg(0, dayOfViolation)
    return 0

def violatedPlTooltip(attachee, args, evt_obj):
    featName = feat_utils.getFeatName(args)
    featKey = featName.upper().replace(" ", "_")
    dayOfViolation = args.get_arg(0)
    daysTillRedemption = (dayOfViolation + 29) - dayOfViolation
    durationString = "days" if daysTillRedemption != 1 else "day"
    evt_obj.append(tpdp.hash(featKey), -2, " ({} until redemption: {})".format(durationString, daysTillRedemption))
    return 0

fifoViolatedPrimalLiving = PythonModifier("Violated Primal Living", 2) #dayOfViolation, empty
fifoViolatedPrimalLiving.AddHook(ET_OnNewDay, EK_NEWDAY_REST, countdownToRedemption, ())
fifoViolatedPrimalLiving.AddHook(ET_OnD20PythonQuery, "PQ_Has_Violated_Primal_Living", feat_utils.queryReturnOne, ())
fifoViolatedPrimalLiving.AddHook(ET_OnGetEffectTooltip, EK_NONE, violatedPlTooltip, ())

### Unarmed Damage
#Your unarmed attacks deal more damage than usual. At 1st level, you deal 1d8 points of damage with each unarmed strike.
#When you attain 3rd level, this damage increases to 1d10 points. See the monk class feature (PH 41).
#If your unarmed attack already deals this amount of damage, increase the base damage to the next step indicated on the monk class table.

#Unarmed damage is set in /templeplus/condition.cpp
#int __cdecl GlobalOnDamage(DispatcherCallbackArgs args)
#At the moment I simplified the monk interaction to increase the damage by one step.
#This is only inaccurate for high monk levels, which are hard to access with the PrC
#As you cannot add more monk levels once you added Fist levels
#Unarmed damage interaction should be more generalized anyways,
#There are more unarmed interactions with different Prc's in the future

### Untamed Strike
#On attaining 2nd level, you can channel the untamed power of nature when you attack.
#Your unarmed strikes are treated as magic weapons; see the monk’s ki strike class feature (PH 41).
#If your unarmed strikes are already magical, they instead are treated lesser ghost touch weapons.
#They deal full damage against incorporeal creatures 50% of the time and half damage the rest of the time.

#I believe Ghost Touch is not properly implemented in this game and I will skip
#that part for now
def addAttackPowerType(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    flags = evt_obj.attack_packet.get_flags()
    if attachee.has_feat(feat_ki_strike):
        evt_obj.bonus_list.add_zeroed(408) #ID 408 NEW!
    elif weaponUsed != OBJ_HANDLE_NULL or flags & D20CAF_RANGED:
        return 0
    else:
        evt_obj.damage_packet.attack_power |= D20DAP_MAGIC
        evt_obj.bonus_list.add_zeroed(407) #ID 407 NEW!
    return 0

fifoUntamedStrike = PythonModifier("Fist of the Forest Untamed Strike", 4) #featEnum, empty
fifoUntamedStrike.MapToFeat("Fist of the Forest Untamed Strike", feat_cond_arg2 = 0)
fifoUntamedStrike.AddHook(ET_OnDealingDamage2, EK_NONE, addAttackPowerType, ())

### Scent
#Scent is not in the game to my knowledge, highly likely to be a skip

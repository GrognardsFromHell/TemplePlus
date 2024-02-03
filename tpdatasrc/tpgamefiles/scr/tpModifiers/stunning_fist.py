from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Stunning Fist Charge Handler"

# Replacing Stunning Fist, so new feats can interact with the Stunning Fist Charges
# This also deactivates Stunning_Attacks as it is not called anymore

stunningFistEnum = 1201

def getFeatName():
    combatMesId = 5047 # ID 5047: Stunning Fist
    return game.get_mesline('mes\\combat.mes', combatMesId)

def getFeatTag():
    return "TAG_STUNNING_FIST"

def getMaxCharges(attachee):
    monkLevel = attachee.stat_level_get(stat_level_monk)
    nonMonkLevel = attachee.stat_level_get(stat_level) - monkLevel
    chargesFromMonk = monkLevel
    chargesNonMonk = nonMonkLevel / 4
    chargesFromFeats = attachee.d20_query("PQ_Get_Extra_Stunning_Fist_Charges")
    return chargesFromMonk + chargesNonMonk + chargesFromFeats

def setStunningFistCharges(attachee, args, evt_obj):
    #100F9820 still collects charges from classes
    print "100F9820 still collects the charges from classes:"
    print "arg.get_arg(0): {}".format(args.get_arg(0))
    maxCharges = getMaxCharges(attachee)
    args.set_arg(0, maxCharges)
    return 0

def deductStunningFistCharge(attachee, args, evt_obj):
    chargesToDeduct = evt_obj.data1
    chargesLeft = args.get_arg(0)
    if chargesLeft:
        chargesLeft -= chargesToDeduct
        args.set_arg(0, chargesLeft)
    return 0

def getStunningFistCharges(attachee, args, evt_obj):
    chargesLeft = args.get_arg(0)
    evt_obj.return_val = chargesLeft
    return 0

def stunningFistRadial(attachee, args, evt_obj):
    maxCharges = getMaxCharges(attachee)
    chargesLeft = args.get_arg(0)
    featName = getFeatName()
    featTag = getFeatTag()
    radialId = tpdp.RadialMenuEntryPythonAction("{} {}/{}".format(featName, chargesLeft, maxCharges), D20A_PYTHON_ACTION, stunningFistEnum, 0, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def stunningFistCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    if attachee.d20_query("PQ_Is_Stunning_Fist_Activated"): #Stunning Fist can be used only once per round
        evt_obj.return_val = AEC_INVALID_ACTION # How difficult would be creating new AEC's? Something like AEC_ALREADY_USED_THIS_TURN would come in handy for some actions
    if not attachee.item_worn_at(item_wear_weapon_primary) == OBJ_HANDLE_NULL:
        evt_obj.return_val = AEC_WRONG_WEAPON_TYPE
    return 0

def stunningFistPerform(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    particlesString = "ft-Stunning Fist"
    particlesID = game.particles(particlesString, attachee)
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
    if attachee.condition_add_with_args("Stunning Fist Effect", particlesID, 0, 0):
        # Deduct Stunning Fist Charge
        chargesToDeduct = 1
        attachee.d20_send_signal("PS_Deduct_Stunning_Fist_Charge", chargesToDeduct)
    return 0

stunningFistExtend = PythonModifier("Stunning_Fist", 2) #override feat_stunning_fist
stunningFistExtend.MapToFeat(feat_stunning_fist)
stunningFistExtend.AddHook(ET_OnConditionAdd, EK_NONE, setStunningFistCharges, ())
stunningFistExtend.AddHook(ET_OnNewDay, EK_NEWDAY_REST, setStunningFistCharges, ())
stunningFistExtend.AddHook(ET_OnD20PythonSignal, "PS_Deduct_Stunning_Fist_Charge", deductStunningFistCharge, ())
stunningFistExtend.AddHook(ET_OnD20PythonQuery, "PQ_Get_Stunning_Fist_Charges", getStunningFistCharges, ())
stunningFistExtend.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, stunningFistRadial, ())
stunningFistExtend.AddHook(ET_OnD20PythonActionCheck, stunningFistEnum, stunningFistCheck, ())
stunningFistExtend.AddHook(ET_OnD20PythonActionPerform, stunningFistEnum, stunningFistPerform, ())

# Stunning Fist Effect

def checkStunImmunity(target):
    stunImmuneMcTypes = [mc_type_construct, mc_type_ooze, mc_type_plant, mc_type_undead]
    for mcType in stunImmuneMcTypes:
        if target.is_category_type(mcType):
            return True
    if target.is_category_subtype(mc_subtype_incorporeal):
        return True
    return False

def triggerStunEffect(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    alreadyTriggered = args.get_arg(1)
    if weaponUsed == OBJ_HANDLE_NULL and not alreadyTriggered:
        target = evt_obj.attack_packet.target
        #There is no query asking if a critter is stun immune
        if not checkStunImmunity(target):
            stunningDc = 10 + (attachee.stat_level_get(stat_level) / 2) + ((attachee.stat_level_get(stat_wisdom) - 10) / 2)
            if target.saving_throw(stunningDc, D20_Save_Fortitude, D20STD_F_NONE, attachee, D20A_NONE): #successful save
                target.float_mesfile_line('mes\\spell.mes', 30001) #ID 30001: Saving throw successful!
            else:
                #arg2 is set to attackerInitiative in 100E8420, not sure if needed
                #was this dolios bug report for stun not getting cleared properly?
                #could also simply set arg2 to 0 
                duration = 1
                attackerInitiative = attachee.get_initiative()
                combatMesId = 89 # ID 89: Stunned!
                target.float_mesfile_line('mes\\combat.mes', combatMesId)
                target.condition_add_with_args("Stunned", duration, attackerInitiative)
        else:
            spellMesId = 32000 # ID 32000: Target is immune!
            target.float_mesfile_line('mes\\spell.mes', spellMesId)
    return 0

def stunningFistActivatedQuery(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

def stunningFistUsed(attachee, args, evt_obj):
    #Set activatedFlag
    args.set_arg(1, 1)
    #Remove Particles
    particlesID = args.get_arg(0)
    game.particles_end(particlesID)
    args.set_arg(0, 0)
    return 0

def removeOnBeginRound(attachee, args, evt_obj):
    if args.get_arg(0): #check if Stunning Fist wasn't used up during the turn and particles still going on
        particlesID = args.get_arg(0)
        game.particles_end(particlesID)
    args.condition_remove()
    return 0

stunningFistEffect = PythonModifier("Stunning Fist Effect", 3) #particlesID, activatedFlag, empty
stunningFistEffect.AddHook(ET_OnDealingDamage, EK_NONE, triggerStunEffect, ())
stunningFistEffect.AddHook(ET_OnD20Signal, EK_S_Attack_Made, stunningFistUsed, ())
stunningFistEffect.AddHook(ET_OnD20PythonQuery, "PQ_Is_Stunning_Fist_Activated", stunningFistActivatedQuery, ())
stunningFistEffect.AddHook(ET_OnBeginRound, EK_NONE, removeOnBeginRound, ())

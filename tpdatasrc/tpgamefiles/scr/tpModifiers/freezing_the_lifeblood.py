from templeplus.pymod import PythonModifier
from toee import *
import tpdp

# Freezing The Lifeblood: Complete Warrior, p. 99

freezingTheLifebloodEnum = 1225

def getFeatName():
    return "Freezing The Lifeblood"

print "Registering {}".format(getFeatName())

def getFeatTag():
    return "TAG_{}".format(getFeatName().upper().replace(" ", "_"))

def createRadial(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    radialData1 = 0
    radialId = tpdp.RadialMenuEntryPythonAction(featName, D20A_PYTHON_ACTION, freezingTheLifebloodEnum, radialData1, featTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Feats)
    return 0

def actionCheck(attachee, args, evt_obj):
    chargesLeft = attachee.d20_query("PQ_Get_Stunning_Fist_Charges")
    if chargesLeft < 1:
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    return 0

def actionPerform(attachee, args, evt_obj):
    featName = getFeatName()
    featTag = getFeatTag()
    particlesString = "ft-{}".format(featName)
    particlesID = game.particles(particlesString, attachee)
    game.create_history_freeform("{} activates ~{}~[{}]\n\n".format(attachee.description, featName, featTag))
    conditionName = "{} Effect".format(featName)
    attachee.condition_add_with_args(conditionName, particlesID, 0)
    return 0

freezingTheLifebloodFeat = PythonModifier("{} Feat".format(getFeatName()), 2) #featEnum, empty
freezingTheLifebloodFeat.MapToFeat("{}".format(getFeatName()), feat_cond_arg2 = 0)
freezingTheLifebloodFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, createRadial, ())
freezingTheLifebloodFeat.AddHook(ET_OnD20PythonActionCheck, freezingTheLifebloodEnum, actionCheck, ())
freezingTheLifebloodFeat.AddHook(ET_OnD20PythonActionPerform, freezingTheLifebloodEnum, actionPerform, ())

# Freezing The Lifeblood Effect
def addExtraDamage(attachee, args, evt_obj):
    weaponUsed = evt_obj.attack_packet.get_weapon_used()
    if weaponUsed == OBJ_HANDLE_NULL:
        #Freezing The Lifeblood deals no damage, so set damage to 0
        damagePacket = evt_obj.damage_packet
        modFactor = 0.0
        damageMesID = 4005 # NEW ID! added in damage_ext.mes
        for damageType in range (D20DT_BLUDGEONING, D20DT_SLASHING_AND_BLUDGEONING_AND_PIERCING + 1):
            if damagePacket.get_overall_damage_by_type(damageType):
                print "damageType: {}, damageAmout: {}".format(damageType, damagePacket.get_overall_damage_by_type(damageType))
                evt_obj.damage_packet.add_mod_factor(modFactor, damageType, damageMesID)
        #Check if target gets stunned
        target = evt_obj.attack_packet.target
        if target.is_category_type(mc_type_humanoid):
            #Check if immune to stun missing
            freezingDc = 10 + (attachee.stat_level_get(stat_level) / 2) + ((attachee.stat_level_get(stat_wisdom) - 10) / 2)
            if target.saving_throw(freezingDc, D20_Save_Fortitude, D20STD_F_NONE, attachee, D20A_NONE): #successful save
                target.float_mesfile_line('mes\\spell.mes', 30001) #ID 30001: Saving throw successful!
            else:
                target.float_mesfile_line('mes\\combat.mes', 149) #ID 149: Paralyzed
                durationDice = dice_new("1d4")
                durationDice.bonus = 1
                duration = durationDice.roll()
                print "duration: {}".format(duration)
                featName = getFeatName()
                particlesString = "ft-{}-Hit".format(featName)
                particlesID = game.particles(particlesString, target)
                game.create_history_freeform("{} is ~paralyzed~[TAG_PARALYZED]\n\n".format(target.description))
                conditionName = "{} Debuff".format(featName)
                target.condition_add_with_args(conditionName, particlesID, duration, 0)
        else:
            target.float_mesfile_line('mes\\spell.mes', 32000) #ID 32000: Target is immune!
    return 0

def removeSignal(attachee, args, evt_obj):
    # Deduct Stunning Fist Charge
    chargesToDeduct = 1
    attachee.d20_send_signal("PS_Deduct_Stunning_Fist_Charge", chargesToDeduct)
    # Remove after use
    args.condition_remove()
    return 0

def removeParticles(attachee, args, evt_obj):
    particlesID = args.get_arg(0)
    game.particles_end(particlesID)
    return 0

def tooltip(attachee, args, evt_obj):
    featName = getFeatName()
    evt_obj.append("{} activated".format(featName))
    return 0

freezingTheLifebloodEffect = PythonModifier("{} Effect".format(getFeatName()), 2) #particlesID, empty
freezingTheLifebloodEffect.AddHook(ET_OnDealingDamage, EK_NONE, addExtraDamage, ())
freezingTheLifebloodEffect.AddHook(ET_OnD20Signal, EK_S_Attack_Made, removeSignal, ())
freezingTheLifebloodEffect.AddHook(ET_OnConditionRemove, EK_NONE, removeParticles, ())
freezingTheLifebloodEffect.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

# Freezing The Lifeblood Debuff

def debuffPreActions(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Remove Paralysis"):
        args.condition_remove()
    return 0

def debuffAddActions(attachee, args, evt_obj):
    duration = args.get_arg(1)
    #Add existing Held condition to handle the helpless status
    attachee.condition_add_with_args("Held", 0, duration, 0)
    return 0

def debuffRemoveActions(attachee, args, evt_obj):
    #Remove Held condition
    conditionReference = tpdp.get_condition_ref("sp-Hold Person")
    attachee.d20_send_signal(S_Spell_End, 5, conditionReference)
    #End particles
    particlesID = args.get_arg(0)
    game.particles_end(particlesID)
    return 0

def debuffTickdown(attachee, args, evt_obj):
    args.set_arg(1, args.get_arg(1)-evt_obj.data1) # Ticking down duration
    if args.get_arg(1) < 0:
        args.condition_remove()
    return 0

def debuffQueryHeld(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

freezingTheLifebloodDebuff = PythonModifier("{} Debuff".format(getFeatName()), 3) #particlesID, duration, empty
freezingTheLifebloodDebuff.AddHook(ET_OnConditionAddPre, EK_NONE, debuffPreActions, ())
freezingTheLifebloodDebuff.AddHook(ET_OnConditionAdd, EK_NONE, debuffAddActions, ())
freezingTheLifebloodDebuff.AddHook(ET_OnD20Query, EK_Q_Critter_Is_Held, debuffQueryHeld, ())
freezingTheLifebloodDebuff.AddHook(ET_OnBeginRound, EK_NONE, debuffTickdown, ())
freezingTheLifebloodDebuff.AddHook(ET_OnConditionRemove, EK_NONE, debuffRemoveActions, ())

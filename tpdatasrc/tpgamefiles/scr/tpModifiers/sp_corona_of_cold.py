from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, applyBonus, applyDamageResistance

print "Registering sp-Corona of Cold"

coronaOfColdSpell = AuraSpellHandleModifier("sp-Corona of Cold", aoe_event_target_all_exclude_self) #spellId, duration, bonusValue, spellEventId, spellDc, empty
coronaOfColdSpell.AddHook(ET_OnTakingDamage2, EK_NONE, applyDamageResistance, (10, D20DT_FIRE,))
coronaOfColdSpell.AddSpellDismiss()

### Start Corona of Cold Effect ###

def coronaOfColdEffectBeginRound(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellDc = args.get_arg(4)
    failedSave = args.get_arg(5)
    spellPacket = tpdp.SpellPacket(spellId)
    game.create_history_freeform("{} is affected by ~Corona of Cold~[TAG_SPELLS_CORONA_OF_COLD]\n\n".format(attachee.description))
    #Saving Throw to negate damage
    if not attachee.saving_throw_spell(spellDc, D20_Save_Fortitude, D20STD_F_SPELL_DESCRIPTOR_COLD, spellPacket.caster, spellId): #success
        attachee.float_text_line("Corona of Cold damage")
        spellDamageDice = dice_new("1d12")
        attachee.spell_damage(spellPacket.caster, D20DT_COLD, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
        if not failedSave: #if not already affected by shiver affect, activate it
            game.create_history_freeform("{} starts to shiver!\n\n".format(attachee.description))
            args.set_arg(5, 1)
    return 0

def addAbilityPenalty(attachee, args, evt_obj):
    failedSave = args.get_arg(5)
    if failedSave:
        applyBonus(attachee, args, evt_obj)
    return 0

def addMovementPenalty(attachee, args, evt_obj):
    failedSave = args.get_arg(5)
    if failedSave:
        moveSpeedBase = attachee.stat_level_get(stat_movement_speed)
        evt_obj.bonus_list.add(-(moveSpeedBase/2), 0 ,"~Corona of Cold~[TAG_SPELLS_CORONA_OF_COLD] Penalty") #Corona of Cold halfs movement speed on a failed save
        #newSpeed = evt_obj.bonus_list.get_sum()
        #if newSpeed < 5:
        #    speedToAdd = 5 - newSpeed
        #    evt_obj.bonus_list.add(speedToAdd, 0, "~Corona of Cold~[TAG_SPELLS_CORONA_OF_COLD] reduces to a minimum of 5 speed")
    return 0

def shiveringTooltip(attachee, args, evt_obj):
    failedSave = args.get_arg(5)
    if failedSave:
        evt_obj.append("Shivering!")
    return 0

def shiveringEffectTooltip(attachee, args, evt_obj):
    failedSave = args.get_arg(5)
    if failedSave:
        idString = "CORONA_OF_COLD_SHIVERING"
        idKey = tpdp.hash(idString)
        evt_obj.append(idKey, -2, "")
    return 0

coronaOfColdEffect = AuraSpellEffectModifier("Corona of Cold", 7) #spellId, duration, bonusValue, spellEventId, spellDc, failedSave, empty
coronaOfColdEffect.AddHook(ET_OnBeginRound, EK_NONE, coronaOfColdEffectBeginRound, ())
coronaOfColdEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_DEXTERITY, addAbilityPenalty, (-2, bonus_type_untyped,))
coronaOfColdEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, addAbilityPenalty, (-2, bonus_type_untyped,))
coronaOfColdEffect.AddHook(ET_OnGetMoveSpeedBase, EK_NONE, addMovementPenalty, ())
coronaOfColdEffect.AddHook(ET_OnGetTooltip, EK_NONE, shiveringTooltip, ()) #special Tooltip for secondary effect
coronaOfColdEffect.AddHook(ET_OnGetEffectTooltip, EK_NONE, shiveringEffectTooltip, ()) #special Tooltip for secondary effect
coronaOfColdEffect.AddSpellDismiss()

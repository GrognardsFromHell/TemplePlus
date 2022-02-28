from toee import *
import tpdp
from spell_utils import AoeObjHandleModifier, AoeSpellEffectModifier, verifyEventId, verifyAoeEventTarget

print "Registering sp-Fugue"

def fugueSpellOnEntered(attachee, args, evt_obj):
    spellTarget = evt_obj.target
    spellId = args.get_arg(0)
    duration = args.get_arg(1)
    spellDc = args.get_arg(2)
    spellEventId = args.get_arg(3)
    spellPacket = tpdp.SpellPacket(spellId)
    aoeEventId = evt_obj.evt_id

    if not verifyEventId(spellEventId, aoeEventId):
        return 0

    if verifyAoeEventTarget(args, spellTarget, spellPacket):

        spellPacket.trigger_aoe_hit()

        conditionName = args.get_cond_name()
        particlesId = game.particles("{}-hit".format(conditionName), spellTarget)

        if spellPacket.add_target(spellTarget, particlesId):
            if spellTarget.saving_throw_spell(spellDc, D20_Save_Fortitude, D20STD_F_NONE, spellPacket.caster, spellId): # save to be only disorientated condition
                spellTarget.float_mesfile_line('mes\\spell.mes', 30001)
                spellTarget.condition_add_with_args("Fugue Disoriented", spellId, duration + 1, spellDc, spellEventId)
            else:
                spellTarget.float_mesfile_line('mes\\spell.mes', 30002)
                spellTarget.condition_add_with_args("Fugue", spellId, duration + 1, spellDc, spellEventId)
                spellPacket.update_registry()
    return 0

fugueSpell = AoeObjHandleModifier("sp-Fugue") #spellId, duration, spellDc, eventId, empty
fugueSpell.AddHook(ET_OnObjectEvent, EK_OnEnterAoE, fugueSpellOnEntered, ())
fugueSpell.AddSpellConcentration()

### Start Fugue Effect ###

def fugueConditionBeginRound(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellCaster = spellPacket.caster
    spellCasterPerformSkill = spellCaster.skill_level_get(skill_perform)
    skillDice = dice_new("1d20")
    skillDiceRoll = skillDice.roll()
    skillRollResult = skillDiceRoll + spellCasterPerformSkill if not attachee.is_friendly(spellCaster) else 0
    print "Perform Check Result", skillRollResult

    game.create_history_freeform("~Fugue~[TAG_SPELLS_FUGUE] effect:\n")
    if skillRollResult < 15:
        attachee.float_text_line("Fugue nothing happened\n\n")
        game.create_history_freeform("Nothing happened")
    elif skillRollResult in range(15, 20):
        spellDamageDice = dice_new('1d6')
        spellDamageDice.number = 3
        attachee.spell_damage(spellCaster, D20DT_SUBDUAL, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    elif skillRollResult in range(20, 25):
        spellDamageDice = dice_new("1d6")
        spellDamageDice.number = 3
        attachee.spell_damage(spellCaster, D20DT_SONIC, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    elif skillRollResult in range(25, 30):
        attachee.fall_down()
        attachee.condition_add("Prone")
        attachee.float_mesfile_line('mes\\combat.mes', 18, 1) #ID18: Knockdown message
        game.create_history_freeform("{} is knocked ~prone~[TAG_PRONE]\n\n".format(attachee.description))
    elif skillRollResult in range(30, 35):
        duration = 1
        persistentFlag = 1
        #game.create_history_freeform("{} is ~nauseated~[TAG_NAUSEATED]\n\n".format(attachee.description)) // Nauseated float is done by the condition itself
        attachee.condition_add_with_args("Nauseated", duration, persistentFlag, 0)
    #elif skillRollResult in range(35, 40):
    else:
        attachee.float_text_line("Stunned", tf_red)
        attachee.condition_add_with_args('Stunned', 1, 0)
        attacheePartsysId = game.particles('sp-Daze2', attachee)
        game.create_history_freeform("{} is ~stunned~[TAG_STUNNED]\n\n".format(attachee.description))
    #else:
        #on a skill check of 41+ Fugue would force the target to attack nearest target
    return 0


fugueCondition = AoeSpellEffectModifier("Fugue") #spellId, duration, spellDc, eventId, empty
fugueCondition.AddHook(ET_OnBeginRound, EK_NONE, fugueConditionBeginRound, ())
fugueCondition.AddSpellConcentration()

### End Fugue Effect ###

### Start Fugue Disoriented ###

fugueDisoriented = AoeSpellEffectModifier("Fugue Disoriented") #spellId, duration, spellDc, eventId, empty
fugueDisoriented.AddToHitBonus(-2, bonus_type_untyped)
fugueDisoriented.AddSkillBonus(-2, bonus_type_untyped)
fugueDisoriented.AddSpellConcentration()

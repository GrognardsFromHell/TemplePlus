from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, spellTime, applyBonus
from pymod_utils import PythonModifier

print "Registering sp-Enervating Shadow"

enervatingShadowSpell = AuraSpellHandleModifier("sp-Enervating Shadow", aoe_event_target_living_creatures_exclude_self) #spellId, duration, empty, spellEventId, spellDc, empty
enervatingShadowSpell.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, applyBonus, (50, bonus_type_concealment,))
enervatingShadowSpell.AddSpellDismiss()
enervatingShadowSpell.AddSpellNoDuplicate()
enervatingShadowSpell.AddDispelledByLight(-1,)

### Start Enervating Shadow Effect ###

def saveAgainstEffect(attachee, args, evt_obj):
    failedSave = args.get_arg(2)
    if not failedSave:
        spellId = args.get_arg(0)
        spellPacket = tpdp.SpellPacket(spellId)
        spellDc = spellPacket.dc
        spellCaster = spellPacket.caster
        saveType = D20_Save_Fortitude
        saveDescriptor = D20STD_F_NONE
        spellTarget = attachee
        if not spellTarget.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId):
            particlesId = game.particles("sp-Enervating Shadow-loss", attachee)
            duration = 4
            args.set_arg(1, duration)
            args.set_arg(2, 1)
            args.set_arg(5, particlesId)
    return 0

def applyStrengthPenalty(attachee, args, evt_obj):
    failedSave = args.get_arg(2)
    if failedSave:
        applyBonus(attachee, args, evt_obj)
    return 0

def applyImmunity(attachee, args, evt_obj):
    failedSave = args.get_arg(2)
    particlesId = args.get_arg(5)
    if failedSave:
        duration = 14400 # 1 day
        attachee.condition_add_with_args("Enervating Shadow Immunity", duration, 0)
    if particlesId:
        game.particles_end(particlesId)
    return 0

enervatingShadowEffect = AuraSpellEffectModifier("Enervating Shadow", 7) #spellId, duration, failedSaveFlag, spellEventId, spellDc, particlesId, empty
enervatingShadowEffect.AddHook(ET_OnBeginRound, EK_NONE, saveAgainstEffect, ())
enervatingShadowEffect.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, applyStrengthPenalty, (-4, bonus_type_untyped,))
enervatingShadowEffect.AddHook(ET_OnConditionRemove, EK_NONE, applyImmunity, ())
enervatingShadowEffect.AddSpellDismiss()
enervatingShadowEffect.AddDispelledByLight(-1,)

### Enervating Shadow Immunity ###

def tooltip(attachee,args, evt_obj):
    duration = args.get_arg(0)
    durationLabel = spellTime(duration)
    conditionName = args.get_cond_name()
    evt_obj.append("{} ({})".format(conditionName, durationLabel))
    return 0

def envShadowImmunity(attachee, args, evt_obj):
    if evt_obj.is_modifier("Enervating Shadow"):
        evt_obj.return_val = 0
    return 0

def tickdown(attachee, args, evt_obj):
    duration = args.get_arg(0)
    duration -= evt_obj.data1
    args.set_arg(0, duration)
    if duration < 0:
        args.condition_remove()
    return 0

enervatingShadowImmunity = PythonModifier("Enervating Shadow Immunity", 2) #duration, empty
enervatingShadowImmunity.AddHook(ET_OnConditionAddPre, EK_NONE, envShadowImmunity, ())
enervatingShadowImmunity.AddHook(ET_OnBeginRound, EK_NONE, tickdown, ())
enervatingShadowImmunity.AddHook(ET_OnGetTooltip, EK_NONE, tooltip, ())

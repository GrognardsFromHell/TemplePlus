from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier

print "Registering sp-Miasmic Cloud"

breathOfTheNightSpell = AoeSpellHandleModifier("sp-Miasmic Cloud") #spellId, duration, bonusValue, spellEventId, spellDc, empty

def applyFatigue(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    if not attachee == spellPacket.caster:
        spellDc = spellPacket.dc
        saveType = D20_Save_Fortitude
        saveDescriptor = D20STD_F_NONE
        if not attachee.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId):
            duration = -1
            attachee.condition_add_with_args("FatigueExhaust", 0, duration, 0, 1, 0, 0)
    return 0

def updateFatigueDuration(attachee, args, evt_obj):
    if attachee.d20_query("Fatigued"):
        duration = 1
        attachee.d20_send_signal("PS_Update_Fatigue_Duration", duration)
    return 0

breathOfTheNightEffect = AoeSpellEffectModifier("Miasmic Cloud") #spellId, duration, bonusValue, spellEventId, spellDc, empty
breathOfTheNightEffect.AddHook(ET_OnConditionAdd, EK_NONE, applyFatigue, ())
breathOfTheNightEffect.AddHook(ET_OnConditionRemove, EK_NONE, updateFatigueDuration, ())
breathOfTheNightEffect.AddFogConcealment()
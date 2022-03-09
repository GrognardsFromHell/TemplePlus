from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier

print "Registering sp-Miasmic Cloud"

breathOfTheNightSpell = AoeSpellHandleModifier("sp-Miasmic Cloud") #spellId, duration, bonusValue, spellEventId, spellDc, empty

def applyFatigue(attachee, args, evt_obj):
    return 0

breathOfTheNightEffect = AoeSpellEffectModifier("sp-Miasmic Cloud") #spellId, duration, bonusValue, spellEventId, spellDc, empty
breathOfTheNightEffect.AddHook(ET_OnConditionAdd, EK_NONE, applyFatigue, ())
breathOfTheNightEffect.AddFogConcealment()
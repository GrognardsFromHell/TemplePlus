from toee import *
import tpdp
from spell_utils import AoeSpellHandleModifier, AoeSpellEffectModifier

print "Registering sp-Breath of the Night"

breathOfTheNightSpell = AoeSpellHandleModifier("sp-Breath of the Night") #spellId, duration, bonusValue, spellEventId, spellDc, empty

breathOfTheNightEffect = AoeSpellEffectModifier("Breath of the Night") #spellId, duration, bonusValue, spellEventId, spellDc, empty
breathOfTheNightEffect.AddFogConcealment()

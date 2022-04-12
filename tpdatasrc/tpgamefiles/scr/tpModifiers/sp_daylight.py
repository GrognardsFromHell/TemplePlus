from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, applyBonus

print "Registering sp-Daylight"

daylightSpell = AuraSpellHandleModifier("sp-Daylight", aoe_event_target_all) #spellId, duration, empty, spellEventId, spellDc, empty
daylightSpell.AddDarknessInteraction()
daylightSpell.AddSpellDismiss()

### Start Daylight Effect ###

daylightEffect = AuraSpellEffectModifier("Daylight") #spellId, duration, empty, spellEventId, spellDc, empty
daylightEffect.AddDarknessInteraction()
daylightEffect.AddSpellDismiss()

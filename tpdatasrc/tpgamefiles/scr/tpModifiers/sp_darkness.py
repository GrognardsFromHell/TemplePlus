from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, applyBonus

print "Registering sp-Darkness"

darknessSpell = AuraSpellHandleModifier("sp-Darkness", aoe_event_target_all) #spellId, duration, empty, spellEventId, spellDc, empty
darknessSpell.AddLightInteraction()
darknessSpell.AddSpellDismiss()

### Start Darkness Effect ###

darknessEffect = AuraSpellEffectModifier("Darkness") #spellId, duration, empty, spellEventId, spellDc, empty
darknessEffect.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, applyBonus, (20, bonus_type_untyped,))
darknessEffect.AddLightInteraction()
darknessEffect.AddSpellDismiss()

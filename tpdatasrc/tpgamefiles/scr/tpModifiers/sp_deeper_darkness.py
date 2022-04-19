from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier, applyBonus

print "Registering sp-Deeper Darkness"

deeperDarknessSpell = AuraSpellHandleModifier("sp-Deeper Darkness", aoe_event_target_all) #spellId, duration, empty, spellEventId, spellDc, empty
deeperDarknessSpell.AddLightInteraction()
deeperDarknessSpell.AddSpellDismiss()

### Start Deeper Darkness Effect ###

deeperDarknessEffect = AuraSpellEffectModifier("Deeper Darkness") #spellId, duration, empty, spellEventId, spellDc, empty
deeperDarknessEffect.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, applyBonus, (20, bonus_type_untyped,))
deeperDarknessEffect.AddLightInteraction()
deeperDarknessEffect.AddSpellDismiss()

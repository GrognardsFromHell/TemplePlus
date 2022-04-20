from toee import *
import tpdp
from spell_utils import AuraSpellHandleModifier, AuraSpellEffectModifier

print "Registering sp-Radiance"

radianceSpell = AuraSpellHandleModifier("sp-Radiance", aoe_event_target_all) #spellId, duration, empty, spellEventId, spellDc, empty
radianceSpell.AddDarknessInteraction()
radianceSpell.AddSpellDismiss()

### Start Radiance Effect ###

def applyDazzled(attachee, args, evt_obj):
    if attachee.is_category_type(mc_type_undead):
        duration = 1
        persistentFlag = 1
        attachee.condition_add_with_args("Dazzled", duration, persistentFlag)
    return 0

def updateDazzled(attachee, args, evt_obj):
    if attachee.is_category_type(my_type_undead):
        durationDice = dice_new("1d6")
        duration = durationDice.roll()
        persistentFlag = 0
        attachee.d20_query(PS_Dazzled_Update_Duration, duration, persistentFlag)
        return 0

radianceEffect = AuraSpellEffectModifier("Radiance") #spellId, duration, empty, spellEventId, spellDc, empty
radianceEffect.AddHook(ET_OnConditionAdd, EK_NONE, applyDazzled, ())
radianceEffect.AddHook(ET_OnConditionRemove, EK_NONE, updateDazzled, ())
radianceEffect.AddDarknessInteraction()
radianceEffect.AddSpellDismiss()

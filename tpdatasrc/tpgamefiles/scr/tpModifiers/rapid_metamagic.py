from templeplus.pymod import PythonModifier
from toee import *
import tpdp

print "Registering Rapid Metamagic"


def RapidMMActionCostMod(attachee, args, evt_obj):
    if evt_obj.d20a.action_type != tpdp.D20ActionType.CastSpell:
        return 0

    if evt_obj.cost_orig.action_cost <= 2: # original is already less than full round
        return 0
    if evt_obj.cost_new.action_cost <= 0: # adjusted amount is already free action
        return 0

    # check if the original spell is standard action or less - if so reduce action cost to standard action
    spData = evt_obj.d20a.spell_data
    spEntry = tpdp.SpellEntry(spData.spell_enum)
    if spEntry.spell_enum == 0:
        return 0

    castingTimeType = spEntry.casting_time
    mmData = spData.get_metamagic_data()
    isQuicken = mmData.get_quicken()

    if isQuicken and not (evt_obj.turnbased_status.flags & TBSF_FreeActionSpellPerformed):
        evt_obj.cost_new.action_cost = 0
        evt_obj.turnbased_status.flags |= TBSF_FreeActionSpellPerformed
        #print "reducing cost to 0"
        return 0

    return 0

rapidMM = PythonModifier("Rapid Metamagic Feat", 2) # args are just-in-case placeholders
rapidMM.MapToFeat("Rapid Metamagic")
rapidMM.AddHook(ET_OnActionCostMod, EK_NONE, RapidMMActionCostMod, ())
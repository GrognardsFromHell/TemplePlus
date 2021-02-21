from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions


print "Registering Rapid Metamagic"


def RapidMMActionCostMod(attachee, args, evt_obj):
    
    if evt_obj.d20a.action_type != tpdp.D20ActionType.CastSpell:
        return 0

    if not game.combat_is_active():
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

    casting_time_type = spEntry.casting_time
    mmData = spData.get_metamagic_data()
    isQuicken = mmData.get_quicken()

    if isQuicken and not (evt_obj.turnbased_status.flags & TBSF_FreeActionSpellPerformed):
        evt_obj.cost_new.action_cost = 0
        evt_obj.turnbased_status.flags |= TBSF_FreeActionSpellPerformed
        #print("RapidMM: reducing cost to 0")
        return 0
    #print('RapidMM restore orig time: ')
    # restore the original spell's casting time
    action_err_code, action_cost_base = tpactions.action_cost_from_spell_casting_time(casting_time_type)
    if action_err_code != AEC_OK: # should only be AEC_OUT_OF_COMBAT...
        return 0
    
    if action_cost_base < evt_obj.cost_orig.action_cost:
        #print('new action cost: ', action_cost_base)
        evt_obj.cost_new.action_cost = action_cost_base
    

    return 0

rapidMM = PythonModifier("Rapid Metamagic Feat", 2) # args are just-in-case placeholders
rapidMM.MapToFeat("Rapid Metamagic")
rapidMM.AddHook(ET_OnActionCostMod, EK_NONE, RapidMMActionCostMod, ())
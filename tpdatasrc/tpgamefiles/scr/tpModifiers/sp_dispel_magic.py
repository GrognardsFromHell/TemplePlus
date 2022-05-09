from toee import *
import tpdp
from spell_utils import SpellBasicPyMod, performDispelCheck

print("Registering sp-Dispel Magic")

# This replaces the vanilla sp-Dispel Magic
# And fixes the AoE dispel bug (skipping last target)
# This condition is used by Dispel Magic and Greater Dispel Magic

def triggerDispel(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    isAoe = args.get_arg(2)
    performDispelCheck(attachee, spellId, isAoe)
    args.remove_spell_mod()
    return 0

# args: spell_id, duration, isAoe
# original spell only has 3 params and last param is used for tracking target mode
dispelMagicSpell = SpellBasicPyMod("sp-Dispel Magic")
dispelMagicSpell.AddSpellNoDuplicate()
dispelMagicSpell.AddHook(ET_OnConditionAdd, EK_NONE, triggerDispel, ())

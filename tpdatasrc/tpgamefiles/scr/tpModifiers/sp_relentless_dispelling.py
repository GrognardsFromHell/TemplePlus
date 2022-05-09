from toee import *
import tpdp
from spell_utils import SpellBasicPyMod, performDispelCheck

print("Registering sp-Relentless Dispelling")

def triggerDispel(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    isAoe = False
    performDispelCheck(attachee, spellId, isAoe)
    return 0

relentlessDispelling = SpellBasicPyMod("sp-Relentless Dispelling") # spell_id, duration, empty
relentlessDispelling.AddSpellNoDuplicate()
relentlessDispelling.AddHook(ET_OnConditionAdd, EK_NONE, triggerDispel, ())
relentlessDispelling.AddHook(ET_OnBeginRound, EK_NONE, triggerDispel, ())
relentlessDispelling.AddSpellTooltips()
relentlessDispelling.AddSpellCountdown()

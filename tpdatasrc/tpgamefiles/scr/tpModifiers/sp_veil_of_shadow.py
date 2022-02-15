from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier, applyBonus

print "Registering sp-Veil of Shadow"

def veilOfShadowSpellCheckDispelCondition(attachee, args, evt_obj):
    #Veil of Shadow is dispelled by daylight
    if game.is_outdoor() and game.is_daytime():
        attachee.float_text_line("Dispelled by daylight")
        args.set_arg(1, -1)
    return 0

veilOfShadowSpell = SpellPythonModifier("sp-Veil of Shadow") # spell_id, duration, empty
veilOfShadowSpell.AddHook(ET_OnBeginRound, EK_NONE, veilOfShadowSpellCheckDispelCondition,())
veilOfShadowSpell.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, applyBonus,(20, bonus_type_concealment,))

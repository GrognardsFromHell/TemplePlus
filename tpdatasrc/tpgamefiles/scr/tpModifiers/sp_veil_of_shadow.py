from toee import *
import tpdp
from spell_utils import SpellPythonModifier, applyBonus

print "Registering sp-Veil of Shadow"

veilOfShadowSpell = SpellPythonModifier("sp-Veil of Shadow") # spell_id, duration, empty
veilOfShadowSpell.AddHook(ET_OnGetDefenderConcealmentMissChance, EK_NONE, applyBonus,(20, bonus_type_concealment,))
veilOfShadowSpell.AddDispelledByLight(3)

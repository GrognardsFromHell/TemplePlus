from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Blessed Aim"

blessedAimSpell = SpellPythonModifier("sp-Blessed Aim") # spell_id, duration, empty
blessedAimSpell.AddToHitBonus(2, bonus_type_morale, D20CAF_RANGED)

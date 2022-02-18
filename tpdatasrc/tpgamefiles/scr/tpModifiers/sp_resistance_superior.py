from toee import *
import tpdp
from utilities import *
from spell_utils import SpellPythonModifier

print "Registering sp-Resistance Superior"

resistanceSuperiorSpell = SpellPythonModifier("sp-Resistance Superior") # spell_id, duration, empty
resistanceSuperiorSpell.AddSaveBonus(6, bonus_type_resistance)
